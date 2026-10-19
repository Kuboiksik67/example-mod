#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

// Variables globales para la gestión del buffering/delay de clics
static bool g_hasBufferedClick = false;
static int g_bufferedFrames = 0;
static const int MAX_SAFE_MARGIN_FRAMES = 16;
static PlayerButton g_bufferedButton = PlayerButton::Jump;
static bool g_isSimulating = false;

class $modify(SafeClickPlayer, PlayerObject) {
    void pushButton(PlayerButton btn) override {
        // Si estamos ejecutando la simulación interna, permitimos el comportamiento normal
        if (g_isSimulating) {
            PlayerObject::pushButton(btn);
            return;
        }

        // Solo procesamos si estamos dentro de un nivel (PlayLayer activo)
        auto playLayer = PlayLayer::get();
        if (!playLayer || this != playLayer->m_player1) {
            PlayerObject::pushButton(btn);
            return;
        }

        // Comprobamos si presionar el botón en este frame exacto causa la muerte
        if (wouldDieWithClick(btn)) {
            // El clic es fatal en este frame: lo retenemos para evaluar frames futuros
            g_hasBufferedClick = true;
            g_bufferedFrames = 0;
            g_bufferedButton = btn;
            log::info("[SafeClick] Clic retenido por riesgo de muerte. Buscando frame seguro...");
            return; // No ejecutamos el clic inmediatamente
        }

        // Si el clic es seguro, se procesa de forma normal
        PlayerObject::pushButton(btn);
    }

    bool wouldDieWithClick(PlayerButton btn) {
        auto playLayer = PlayLayer::get();
        if (!playLayer) return false;

        // Flag para evitar bucles infinitos en la simulación
        g_isSimulating = true;

        // Copiamos/guardamos el estado crítico actual del jugador
        CCPoint originalPos = m_position;
        CCPoint originalVel = m_yVelocity;
        bool originalIsDead = m_isDead;
        bool originalOnGround = m_isOnGround;

        // Simulamos la presión del botón
        PlayerObject::pushButton(btn);

        // Simulamos la actualización del jugador en el frame
        // (En GD la física principal se procesa en el update del jugador/nivel)
        this->update(0.016666f); // ~1 frame a 60Hz / tick de física

        bool died = m_isDead;

        // Restauramos el estado original del jugador tras la prueba
        m_position = originalPos;
        m_yVelocity = originalVel;
        m_isDead = originalIsDead;
        m_isOnGround = originalOnGround;

        g_isSimulating = false;
        return died;
    }
};

class $modify(SafeClickPlayLayer, PlayLayer) {
    void update(float dt) override {
        // Ejecutamos la actualización normal del juego primero
        PlayLayer::update(dt);

        // Si hay un clic pendiente de ejecución seguro
        if (g_hasBufferedClick && m_player1 && !m_player1->m_isDead) {
            g_bufferedFrames++;

            // Comprobamos si el frame actual ya es seguro para ejecutar el clic
            auto player = static_cast<SafeClickPlayer*>(m_player1);
            
            if (!player->wouldDieWithClick(g_bufferedButton)) {
                // Encontramos un frame seguro dentro de la ventana de margen
                log::info("[SafeClick] Frame seguro encontrado tras {} frames. Ejecutando clic.", g_bufferedFrames);
                
                g_hasBufferedClick = false;
                m_player1->pushButton(g_bufferedButton);
                return;
            }

            // Si superamos los 16 frames de margen y no hubo frame seguro, soltamos el clic
            if (g_bufferedFrames >= MAX_SAFE_MARGIN_FRAMES) {
                log::info("[SafeClick] Margen de 16 frames agotado. Forzando clic.");
                g_hasBufferedClick = false;
                m_player1->pushButton(g_bufferedButton);
            }
        }
    }

    void resetLevel() override {
        // Reiniciamos las variables de control al reiniciar o morir en el nivel
        PlayLayer::resetLevel();
        g_hasBufferedClick = false;
        g_bufferedFrames = 0;
    }
};
