
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class AILearnerPopup : public geode::Popup {
protected:
    CCLabelBMFont* m_status = nullptr;
    CCLabelBMFont* m_info = nullptr;
    CCLabelBMFont* m_modules = nullptr;

    bool init() {
        if (!Popup::init(300.f, 220.f)) {
            return false;
        }

        this->setTitle("GD AI LEARNER");

        // Glowny status
        m_status = CCLabelBMFont::create(
            "AI STATUS: NOT STARTED",
            "bigFont.fnt"
        );

        if (!m_status) {
            return false;
        }

        m_status->setScale(0.45f);
        m_mainLayer->addChildAtPosition(
            m_status,
            Anchor::Center,
            ccp(0.f, 48.f)
        );

        // Informacja o sesji
        m_info = CCLabelBMFont::create(
            "System ready. Waiting for start.",
            "bigFont.fnt"
        );

        if (!m_info) {
            return false;
        }

        m_info->setScale(0.30f);
        m_mainLayer->addChildAtPosition(
            m_info,
            Anchor::Center,
            ccp(0.f, 18.f)
        );

        // Status przyszlych modulow
        m_modules = CCLabelBMFont::create(
            "Scanner: OFF  |  Memory: OFF",
            "bigFont.fnt"
        );

        if (!m_modules) {
            return false;
        }

        m_modules->setScale(0.32f);
        m_mainLayer->addChildAtPosition(
            m_modules,
            Anchor::Center,
            ccp(0.f, -12.f)
        );

        // Przycisk uruchamiania
        auto sprite = ButtonSprite::create("START AI");

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(AILearnerPopup::onStartAI)
        );

        m_buttonMenu->addChildAtPosition(
            button,
            Anchor::Bottom,
            ccp(0.f, 25.f)
        );

        return true;
    }

    void onStartAI(CCObject*) {
        if (m_status) {
            m_status->setString("AI STATUS: INITIALIZED");
        }

        if (m_info) {
            m_info->setString(
                "Session initialized successfully."
            );
        }

        if (m_modules) {
            m_modules->setString(
                "Scanner: WAITING  |  Memory: WAITING"
            );
        }

        log::info("GD AI Learner: Session initialized");
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

        auto sprite = CCSprite::createWithSpriteFrameName(
            "GJ_likeBtn_001.png"
        );

        if (!sprite) {
            log::error("GD AI Learner: button sprite not found");
            return true;
        }

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(MyMenuLayer::onAILearner)
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
