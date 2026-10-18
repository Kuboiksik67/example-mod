#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <algorithm>
#include <array>
#include <string>

using namespace geode::prelude;

namespace {
constexpr int PROFILE_COUNT = 4;
constexpr char const* PROFILE_SCHEMA_KEY = "ai-profile-schema";
constexpr int PROFILE_SCHEMA_VERSION = 1;

// First playable prototype: automatic, timed cube jumps.
// This is deliberately simple and does not yet recognize obstacles.
bool g_aiEnabled = false;
bool g_jumpHeld = false;
float g_jumpReleaseTimer = 0.f;
float g_jumpCooldown = 0.f;
}

class AILearnerPopup : public geode::Popup {
protected:
    CCLabelBMFont* m_profile = nullptr;
    CCLabelBMFont* m_hardest = nullptr;
    CCLabelBMFont* m_stats = nullptr;
    CCLabelBMFont* m_status = nullptr;

    static std::string key(std::string const& field, int index) {
        return "ai-" + std::to_string(index) + "-" + field;
    }

    static int active() {
        return std::clamp(
            Mod::get()->getSavedValue<int>("ai-active", 0),
            0, PROFILE_COUNT - 1
        );
    }

    static void saveActive(int index) {
        Mod::get()->setSavedValue<int>(
            "ai-active", std::clamp(index, 0, PROFILE_COUNT - 1)
        );
    }

    static void ensureProfiles() {
        auto mod = Mod::get();

        // Keep existing profile data, but guarantee exactly four selectable slots.
        for (int i = 0; i < PROFILE_COUNT; ++i) {
            if (mod->getSavedValue<std::string>(key("name", i), "").empty()) {
                mod->setSavedValue<std::string>(
                    key("name", i), "AI " + std::to_string(i + 1)
                );
            }
            if (mod->getSavedValue<std::string>(key("hardest", i), "").empty()) {
                mod->setSavedValue<std::string>(key("hardest", i), "None");
            }
            // Defaults are only used when values have never been saved.
            // Existing values are not overwritten.
            (void)mod->getSavedValue<int>(key("attempts", i), 0);
            (void)mod->getSavedValue<int>(key("experience", i), 0);
        }

        mod->setSavedValue<int>("ai-count", PROFILE_COUNT);
        saveActive(active());
        mod->setSavedValue<int>(PROFILE_SCHEMA_KEY, PROFILE_SCHEMA_VERSION);
    }

    bool init() {
        if (!Popup::init(360.f, 330.f)) {
            return false;
        }

        ensureProfiles();
        setTitle("GD AI LEARNER");

        m_profile = CCLabelBMFont::create("AI 1", "bigFont.fnt");
        m_hardest = CCLabelBMFont::create("Hardest: None", "bigFont.fnt");
        m_stats = CCLabelBMFont::create("Attempts: 0 | XP: 0", "bigFont.fnt");
        m_status = CCLabelBMFont::create("Select a profile", "bigFont.fnt");

        if (!m_profile || !m_hardest || !m_stats || !m_status) {
            return false;
        }

        // Use a wider, flatter type treatment so the metadata stays readable
        // without being covered by the profile buttons.
        m_profile->setScaleX(0.54f);
        m_profile->setScaleY(0.38f);
        m_hardest->setScaleX(0.40f);
        m_hardest->setScaleY(0.27f);
        m_stats->setScaleX(0.37f);
        m_stats->setScaleY(0.25f);
        m_status->setScaleX(0.33f);
        m_status->setScaleY(0.23f);

        // Move all metadata upward. The lowest line now sits above the first
        // row of slot buttons instead of being hidden behind them.
        m_mainLayer->addChildAtPosition(
            m_profile, Anchor::Center, ccp(0.f, 104.f)
        );
        m_mainLayer->addChildAtPosition(
            m_hardest, Anchor::Center, ccp(0.f, 83.f)
        );
        m_mainLayer->addChildAtPosition(
            m_stats, Anchor::Center, ccp(0.f, 64.f)
        );
        m_mainLayer->addChildAtPosition(
            m_status, Anchor::Center, ccp(0.f, 45.f)
        );

        // Fixed 2x2 grid: four profiles, no create/delete controls.
        addButton("AI SLOT 1", menu_selector(AILearnerPopup::onSlot1), -78.f, 180.f);
        addButton("AI SLOT 2", menu_selector(AILearnerPopup::onSlot2),  78.f, 180.f);
        addButton("AI SLOT 3", menu_selector(AILearnerPopup::onSlot3), -78.f, 143.f);
        addButton("AI SLOT 4", menu_selector(AILearnerPopup::onSlot4),  78.f, 143.f);

        addButton("OPTIONS", menu_selector(AILearnerPopup::onOptions), -78.f, 42.f);
        addButton("START AI", menu_selector(AILearnerPopup::onStartAI), 78.f, 42.f);

        refresh();
        return true;
    }

    void addButton(char const* text, SEL_MenuHandler callback, float x, float y) {
        auto sprite = ButtonSprite::create(text);
        auto button = CCMenuItemSpriteExtra::create(sprite, this, callback);
        m_buttonMenu->addChildAtPosition(
            button, Anchor::Bottom, ccp(x, y)
        );
    }

    void selectSlot(int index) {
        saveActive(index);
        m_status->setString(
            ("Selected AI SLOT " + std::to_string(index + 1)).c_str()
        );
        refresh();
    }

    void refresh() {
        int index = active();
        auto mod = Mod::get();

        auto name = mod->getSavedValue<std::string>(key("name", index), "AI");
        auto hardest = mod->getSavedValue<std::string>(key("hardest", index), "None");
        int attempts = mod->getSavedValue<int>(key("attempts", index), 0);
        int experience = mod->getSavedValue<int>(key("experience", index), 0);

        m_profile->setString(
            (name + "  [" + std::to_string(index + 1) + "/4]").c_str()
        );
        m_hardest->setString(("Hardest: " + hardest).c_str());
        m_stats->setString(
            ("Attempts: " + std::to_string(attempts) +
             " | XP: " + std::to_string(experience)).c_str()
        );
    }

    void onSlot1(CCObject*) { selectSlot(0); }
    void onSlot2(CCObject*) { selectSlot(1); }
    void onSlot3(CCObject*) { selectSlot(2); }
    void onSlot4(CCObject*) { selectSlot(3); }

    void onOptions(CCObject*) {
        FLAlertLayer::create(
            "GD AI Learner — Options",
            "Target game: Geometry Dash 2.2081\n"
            "Geode: 5.11.1\n"
            "Control mode: Autonomous\n"
            "Supported mode planned first: Cube\n\n"
            "The autonomous gameplay engine is not connected in this build. "
            "This screen currently manages profile data only.",
            "OK"
        )->show();
    }

    void onStartAI(CCObject*) {
        g_aiEnabled = !g_aiEnabled;
        g_jumpHeld = false;
        g_jumpReleaseTimer = 0.f;
        g_jumpCooldown = 0.f;

        if (g_aiEnabled) {
            m_status->setString("Auto-jump prototype: ON");
            FLAlertLayer::create(
                "GD AI Learner",
                "AUTO-JUMP PROTOTYPE ENABLED\n\n"
                "Now enter a normal level. The mod will automatically tap jump "
                "at intervals while the cube is on the ground.\n\n"
                "Important: this first test does NOT detect obstacles yet, so it "
                "will not reliably complete a level. Press START AI again to turn it off.",
                "OK"
            )->show();
            log::info("GD AI Learner: auto-jump prototype enabled for profile {}.", active() + 1);
        } else {
            m_status->setString("Auto-jump prototype: OFF");
            FLAlertLayer::create("GD AI Learner", "Auto-jump has been turned off.", "OK")->show();
            log::info("GD AI Learner: auto-jump prototype disabled.");
        }
    }

public:
    static AILearnerPopup* create() {
        auto ret = new AILearnerPopup();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto menu = this->getChildByID("bottom-menu");
        if (!menu) {
            log::error("GD AI Learner: bottom-menu not found");
            return true;
        }

        auto sprite = CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png");
        if (!sprite) {
            log::error("GD AI Learner: button sprite not found");
            return true;
        }

        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(MyMenuLayer::onAILearner)
        );
        button->setID("ai-learner-button"_spr);
        menu->addChild(button);
        menu->updateLayout();
        return true;
    }

    void onAILearner(CCObject*) {
        auto popup = AILearnerPopup::create();
        if (popup) {
            popup->show();
        }
    }
};


// Basic proof that the mod can send jump input to the real player object.
// Obstacle recognition and learning are the next steps after this is built/tested.
class $modify(AILearnerPlayerObject, PlayerObject) {
    void update(float dt) {
        auto playLayer = PlayLayer::get();
        bool isMainPlayer = playLayer && playLayer->m_player1 == this;

        if (isMainPlayer && g_aiEnabled) {
            g_jumpCooldown = std::max(0.f, g_jumpCooldown - dt);
            g_jumpReleaseTimer = std::max(0.f, g_jumpReleaseTimer - dt);

            if (m_isDead || m_controlsDisabled) {
                if (g_jumpHeld) {
                    this->releaseButton(PlayerButton::Jump);
                    g_jumpHeld = false;
                }
            } else {
                // Release the jump shortly after the press so it behaves like a tap.
                if (g_jumpHeld && g_jumpReleaseTimer <= 0.f) {
                    this->releaseButton(PlayerButton::Jump);
                    g_jumpHeld = false;
                }

                // The current prototype is Cube-only: other vehicle flags must be false.
                bool isCube = !m_isShip && !m_isBird && !m_isBall && !m_isDart &&
                              !m_isRobot && !m_isSpider && !m_isSwing;
                if (isCube && m_isOnGround && !g_jumpHeld && g_jumpCooldown <= 0.f) {
                    this->pushButton(PlayerButton::Jump);
                    g_jumpHeld = true;
                    g_jumpReleaseTimer = 0.045f;
                    g_jumpCooldown = 0.34f;
                }
            }
        }

        PlayerObject::update(dt);
    }
};
