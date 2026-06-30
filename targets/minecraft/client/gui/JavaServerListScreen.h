#pragma once
#include <string>
#include <vector>

#include "Screen.h"
#include "app/common/src/JavaEdition/JavaServerList.h"
#include "app/common/src/JavaEdition/JavaServerListStore.h"
#include "app/common/src/JavaEdition/JavaServerPinger.h"

class EditBox;
class Button;
class JavaServerListScreen : public Screen {
public:
    explicit JavaServerListScreen(Screen* lastScreen);
    ~JavaServerListScreen();

    void init() override;
    void tick() override;
    void render(int xm, int ym, float a) override;
    void removed() override;

protected:
    void buttonClicked(Button* button) override;
    void keyPressed(wchar_t ch, int eventKey) override;
    void mouseClicked(int x, int y, int buttonNum) override;
    void tabPressed() override;

private:
    enum class Mode { List, AddEdit };
    enum ButtonId {
        ID_CONNECT = 1,
        ID_ADD = 2,
        ID_EDIT = 3,
        ID_DELETE = 4,
        ID_REFRESH = 5,
        ID_BACK = 6,
        ID_SAVE = 10,
        ID_CANCEL = 11,
    };

    Screen* m_lastScreen;
    JavaServerListStore m_store;
    JavaServerList m_list;
    JavaServerPinger* m_pinger;
    std::vector<PingResult> m_results;
    std::vector<int> m_pingIds;
    int m_nextPingId = 1;

    Mode m_mode = Mode::List;
    int m_selected = -1;
    int m_editIndex = -1;
    bool m_corruptNotice = false;
    std::wstring m_statusMessage;
    bool m_pendingModeSwitch = false;
    Mode m_pendingMode = Mode::List;
    int m_pendingEditIndex = -1;

    EditBox* m_nameEdit = nullptr;
    EditBox* m_addrEdit = nullptr;

    void rebuildButtons();
    void updateButtonStates();
    void clearEditBoxes();
    void enterListMode();
    void enterAddMode();
    void enterEditMode(int index);
    void applyPendingModeSwitch();
    void commitAddEdit();
    void deleteSelected();
    void connectSelected();
    void refreshPings();

    int listTop() const;
    int rowHeight() const;
    int maxVisibleRows() const;

    std::wstring formatStatusLine(int index) const;
};