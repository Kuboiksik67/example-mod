
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class AILearnerPopup : public geode::Popup {
protected:
    CCLabelBMFont* m_statusLabel = nullptr;

    bool init() {
        if (!Popup::init(260.f, 170.f)) {
            return false;
        }

        this->setTitle("GD AI Learner");

        auto status = CCLabelBMFont::create(
            "Status: Not started",
            "bigFont.fnt"
        );

        if (!status) {
            return false;
        }

        status->setScale(0.45f);
        m_mainLayer->addChildAtPosition(
            status,
            Anchor::Center,
            ccp(0.f, 15.f)
        );

        m_statusLabel = status;

        auto startSprite = ButtonSprite::create("START AI");

        auto startButton = CCMenuItemSpriteExtra::create(
            startSprite,
            this,
            menu_selector(AILearnerPopup::onStartAI)
        );

        m_buttonMenu->addChildAtPosition(
            startButton,
            Anchor::Bottom,
            ccp(0.f, 25.f)
        );

        return true;
    }

    void onStartAI(CCObject*) {
        if (m_statusLabel) {
            m_statusLabel->setString(
                "AI Started! (Prototype)"
            );
        }

        log::info("GD AI Learner: Start AI pressed");
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
