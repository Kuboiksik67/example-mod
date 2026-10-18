
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <algorithm>
#include <string>

using namespace geode::prelude;

class AILearnerPopup : public geode::Popup {
protected:
    CCLabelBMFont* m_profile = nullptr;
    CCLabelBMFont* m_hardest = nullptr;
    CCLabelBMFont* m_stats = nullptr;
    CCLabelBMFont* m_status = nullptr;

    static std::string key(std::string const& field, int index) {
        return "ai-" + std::to_string(index) + "-" + field;
    }

    static int count() {
        return Mod::get()->getSavedValue<int>("ai-count", 0);
    }

    static int active() {
        return Mod::get()->getSavedValue<int>("ai-active", 0);
    }

    static void saveActive(int index) {
        Mod::get()->setSavedValue<int>("ai-active", index);
    }

    static void createProfile(int index) {
        Mod::get()->setSavedValue<std::string>(
            key("name", index), "AI " + std::to_string(index + 1)
        );
        Mod::get()->setSavedValue<std::string>(
            key("hardest", index), "None"
        );
        Mod::get()->setSavedValue<int>(
            key("attempts", index), 0
        );
        Mod::get()->setSavedValue<int>(
            key("experience", index), 0
        );
    }

    static void ensureProfiles() {
        if (count() <= 0) {
            createProfile(0);
            Mod::get()->setSavedValue<int>("ai-count", 1);
            saveActive(0);
        }
    }

    bool init() {
        if (!Popup::init(320.f, 300.f)) {
            return false;
        }

        ensureProfiles();
        setTitle("GD AI LEARNER");

        m_profile = CCLabelBMFont::create(
            "AI PROFILE", "bigFont.fnt"
        );
        m_hardest = CCLabelBMFont::create(
            "Hardest: None", "bigFont.fnt"
        );
        m_stats = CCLabelBMFont::create(
            "Attempts: 0 | Experience: 0", "bigFont.fnt"
        );
        m_status = CCLabelBMFont::create(
            "Status: Profile ready", "bigFont.fnt"
        );

        if (!m_profile || !m_hardest ||
            !m_stats || !m_status) {
            return false;
        }

        m_profile->setScale(0.55f);
        m_hardest->setScale(0.42f);
        m_stats->setScale(0.34f);
        m_status->setScale(0.34f);

        m_mainLayer->addChildAtPosition(
            m_profile, Anchor::Center, ccp(0.f, 72.f)
        );
        m_mainLayer->addChildAtPosition(
            m_hardest, Anchor::Center, ccp(0.f, 42.f)
        );
        m_mainLayer->addChildAtPosition(
            m_stats, Anchor::Center, ccp(0.f, 12.f)
        );
        m_mainLayer->addChildAtPosition(
            m_status, Anchor::Center, ccp(0.f, -15.f)
        );

        addButton("PREV", menu_selector(AILearnerPopup::onPrev),
                  -82.f, 76.f);
        addButton("NEXT", menu_selector(AILearnerPopup::onNext),
                   82.f, 76.f);
        addButton("NEW AI", menu_selector(AILearnerPopup::onNewAI),
                  -63.f, 44.f);
        addButton("DELETE", menu_selector(AILearnerPopup::onDelete),
                   63.f, 44.f);
        addButton("START AI", menu_selector(AILearnerPopup::onStartAI),
                    0.f, 13.f);

        refresh();
        return true;
    }

    void addButton(
        char const* text,
        SEL_MenuHandler callback,
        float x,
        float y
    ) {
        auto sprite = ButtonSprite::create(text);
        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, callback
        );

        m_buttonMenu->addChildAtPosition(
            button, Anchor::Bottom, ccp(x, y)
        );
    }

    void refresh() {
        int total = count();
        if (total < 1) return;

        int index = std::clamp(active(), 0, total - 1);
        saveActive(index);

        auto mod = Mod::get();

        auto name = mod->getSavedValue<std::string>(
            key("name", index), "AI"
        );
        auto hardest = mod->getSavedValue<std::string>(
            key("hardest", index), "None"
        );
        int attempts = mod->getSavedValue<int>(
            key("attempts", index), 0
        );
        int experience = mod->getSavedValue<int>(
            key("experience", index), 0
        );

        m_profile->setString(
            (name + "  [" + std::to_string(index + 1) +
             "/" + std::to_string(total) + "]").c_str()
        );

        m_hardest->setString(
            ("Hardest: " + hardest).c_str()
        );

        m_stats->setString(
            ("Attempts: " + std::to_string(attempts) +
             " | XP: " + std::to_string(experience)).c_str()
        );
    }

    void onPrev(CCObject*) {
        int index = active();
        if (index > 0) {
            saveActive(index - 1);
        }
        m_status->setString("Profile selected");
        refresh();
    }

    void onNext(CCObject*) {
        int index = active();
        if (index + 1 < count()) {
            saveActive(index + 1);
        }
        m_status->setString("Profile selected");
        refresh();
    }

    void onNewAI(CCObject*) {
        int index = count();

        createProfile(index);
        Mod::get()->setSavedValue<int>("ai-count", index + 1);
        saveActive(index);

        m_status->setString("New AI created");
        refresh();
    }

    void onDelete(CCObject*) {
        if (count() <= 1) {
            FLAlertLayer::create(
                "GD AI Learner",
                "Keep at least one AI profile.",
                "OK"
            )->show();
            return;
        }

        createQuickPopup(
            "Delete AI",
            "Delete this profile and its saved data?",
            "Cancel",
            "Delete",
            [this](auto, bool confirmed) {
                if (confirmed) {
                    deleteActiveProfile();
                }
            }
        );
    }

    void deleteActiveProfile() {
        auto mod = Mod::get();
        int total = count();
        int removed = active();

        for (int i = removed; i < total - 1; i++) {
            int next = i + 1;

            mod->setSavedValue<std::string>(
                key("name", i),
                mod->getSavedValue<std::string>(
                    key("name", next), "AI"
                )
            );
            mod->setSavedValue<std::string>(
                key("hardest", i),
                mod->getSavedValue<std::string>(
                    key("hardest", next), "None"
                )
            );
            mod->setSavedValue<int>(
                key("attempts", i),
                mod->getSavedValue<int>(
                    key("attempts", next), 0
                )
            );
            mod->setSavedValue<int>(
                key("experience", i),
                mod->getSavedValue<int>(
                    key("experience", next), 0
                )
            );
        }

        mod->setSavedValue<int>("ai-count", total - 1);

        int newIndex = std::min(removed, total - 2);
        saveActive(newIndex);

        m_status->setString("Profile deleted");
        refresh();
    }

    void onStartAI(CCObject*) {
        m_status->setString(
            "AI engine not connected yet"
        );

        log::info(
            "GD AI Learner: Selected AI profile {}",
            active() + 1
        );
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
