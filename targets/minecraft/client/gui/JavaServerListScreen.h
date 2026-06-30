#pragma once
#include <string>
#include <vector>

#include "Screen.h"
#include "app/common/src/JavaEdition/JavaServerList.h"
#include "app/common/src/JavaEdition/JavaServerListStore.h"
#include "app/common/src/JavaEdition/JavaServerPinger.h"

class EditBox;
class Button;

// 4jcraft - Java Edition Servers feature module (Phase 1 MVP).
//
// Browser screen for Minecraft Java Edition 1.8 servers, opened from the
// multiplayer menu. It is a standalone Screen subclass: it does NOT inherit
// from, share code with, or modify JoinMultiplayerScreen (Req 1.4 / 20.3), and
// it is fully isolated from the LCE networking core. This file MUST NOT
// #include Connection.h / Packet.h / PlayerConnection.h / Socket.h and MUST NOT
// use g_NetworkManager or Packet::writePacket (Req 20.1). All Java networking
// is reached only through the JavaEdition module (JavaServerListStore,
// JavaServerList / JavaServerEntry, JavaServerPinger, parseAddress and, for the
// Phase 1 Connect probe, JavaRawTcpClient with its own VarInt frame writer).
//
// The screen has two sub-modes (Req 2): a List mode that renders each saved
// server with its live MOTD / online / max / ping plus a "Querying" indicator
// while a ping is in flight, and an AddEdit mode with name + address edit
// boxes. Pings are polled on the UI thread in tick() via the asynchronous
// JavaServerPinger so the render thread never blocks (Req 4.9), and all ping
// workers are stopped in removed() (Req 4.8 / 20.1).
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
    // Two simple sub-modes for the browser (Req 2): the list of saved servers
    // and the add/edit form.
    enum class Mode { List, AddEdit };

    // Stable button ids per mode (kept disjoint so a stale click can't be
    // misrouted across a mode switch).
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

    Screen* m_lastScreen;          // screen to return to on Back (Req 1.3)
    JavaServerListStore m_store;   // persistence to javaservers.txt (Req 3)
    JavaServerList m_list;         // in-memory model (Req 2)
    JavaServerPinger* m_pinger;    // owned; cancelled in removed() (Req 4)
    std::vector<PingResult> m_results;  // per-entry ping snapshot, parallel to m_list
    std::vector<int> m_pingIds;    // per-entry in-flight ping id (-1 = none)
    int m_nextPingId = 1;          // monotonically increasing ping generation id

    Mode m_mode = Mode::List;
    int m_selected = -1;           // selected list index, -1 if none
    int m_editIndex = -1;          // index being edited in AddEdit, -1 = adding
    bool m_corruptNotice = false;  // set when load() saw corrupt lines (Req 3.6)
    std::wstring m_statusMessage;  // transient notice (e.g. connect error, Req 5.3)

    // Mode switches are deferred out of button handlers: Screen::mouseClicked
    // iterates `buttons` while dispatching, so the actual rebuild happens at
    // the next tick() to avoid invalidating that iterator.
    bool m_pendingModeSwitch = false;
    Mode m_pendingMode = Mode::List;
    int m_pendingEditIndex = -1;

    EditBox* m_nameEdit = nullptr;
    EditBox* m_addrEdit = nullptr;

    // Mode / widget management.
    void rebuildButtons();         // (re)creates the buttons for m_mode
    void updateButtonStates();     // enables/disables selection-dependent buttons
    void clearEditBoxes();         // deletes the AddEdit edit boxes
    void enterListMode();
    void enterAddMode();           // Req 2.1
    void enterEditMode(int index); // Req 2.5
    void applyPendingModeSwitch(); // performs the deferred rebuild in tick()
    void commitAddEdit();          // parses address, mutates list, persists (Req 2.1/2.5/2.7)
    void deleteSelected();         // Req 2.6
    void connectSelected();        // Phase 1 connect probe (Req 5.1/5.3/5.4)

    // Starts a fresh Server List Ping for every saved entry (Req 4.1). Called on
    // entering the list and after any mutation so results stay aligned to the
    // current list order.
    void refreshPings();

    // Shared list-row geometry, used identically by render() and mouseClicked()
    // so hit-testing matches what is drawn.
    int listTop() const;
    int rowHeight() const;
    int maxVisibleRows() const;

    // Renders one line summarising an entry's ping state (Req 4.3/4.5/4.8).
    std::wstring formatStatusLine(int index) const;
};
