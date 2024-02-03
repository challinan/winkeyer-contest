#if 0
#include "sqlite3-connector.h"

QString Sqlite3_connector::getName() {return opname;}
QString Sqlite3_connector::getCallSign() {return callSign;}
QString Sqlite3_connector::getGridSquare() {return gridSquare;}
QString Sqlite3_connector::getCity() {return city;}
QString Sqlite3_connector::getState() {return state;}
QString Sqlite3_connector::getCountry() {return country;}
QString Sqlite3_connector::getCounty() {return county;}
QString Sqlite3_connector::getSerialPort() {return serial_port;}
QString Sqlite3_connector::getArrlSection() {return arrl_section;}


Event Code from MainWindow::event() override

    switch (ev->type()) {

    case QEvent::None: s = "QEvent::None"; break;
    case QEvent::ActionAdded: s = "QEvent::ActionAdded"; break;
    case QEvent::ActionChanged: s = "QEvent::ActionChanged"; break;
    case QEvent::ActionRemoved: s = "QEvent::ActionRemoved"; break;
    case QEvent::ActivationChange: s = "QEvent::ActivationChange"; break;
    case QEvent::ApplicationActivate: s = "QEvent::ApplicationActivate"; break;
    case QEvent::ApplicationDeactivate: s = "QEvent::ApplicationDeactivate"; break;
    case QEvent::ApplicationFontChange: s = "QEvent::ApplicationFontChange"; break;
    case QEvent::ApplicationLayoutDirectionChange: s = "QEvent::ApplicationLayoutDirectionChange"; break;
    case QEvent::ApplicationPaletteChange: s = "QEvent::ApplicationPaletteChange"; break;
    case QEvent::ApplicationStateChange: s = "QEvent::ApplicationStateChange"; break;
    case QEvent::ApplicationWindowIconChange: s = "QEvent::ApplicationWindowIconChange"; break;
    case QEvent::ChildAdded: s = "QEvent::ChildAdded"; break;
    case QEvent::ChildPolished:
        return QWidget::event(ev);
        break;

    case QEvent::ChildRemoved: s = "QEvent::ChildRemoved"; break;
    case QEvent::Clipboard: s = "QEvent::Clipboard"; break;
    case QEvent::Close: s = "QEvent::Close"; break;
    case QEvent::CloseSoftwareInputPanel: s = "QEvent::CloseSoftwareInputPanel"; break;
    case QEvent::ContentsRectChange: s = "QEvent::ContentsRectChange"; break;
    case QEvent::ContextMenu: s = "QEvent::ContextMenu"; break;
    case QEvent::CursorChange: s = "QEvent::CursorChange"; break;
    case QEvent::DeferredDelete: s = "QEvent::DeferredDelete"; break;
    case QEvent::DragEnter: s = "QEvent::DragEnter"; break;
    case QEvent::DragLeave: s = "QEvent::DragLeave"; break;
    case QEvent::DragMove: s = "QEvent::DragMove"; break;
    case QEvent::Drop: s = "QEvent::Drop"; break;
    case QEvent::DynamicPropertyChange: s = "QEvent::DynamicPropertyChange"; break;
    case QEvent::EnabledChange: s = "QEvent::EnabledChange"; break;
    case QEvent::Enter: s = "QEvent::Enter"; break;
    case QEvent::EnterWhatsThisMode: s = "QEvent::EnterWhatsThisMode"; break;
    case QEvent::Expose: s = "QEvent::Expose"; break;
    case QEvent::FileOpen: s = "QEvent::FileOpen"; break;
    case QEvent::FocusIn: s = "QEvent::FocusIn"; break;
    case QEvent::FocusOut: s = "QEvent::FocusOut"; break;
    case QEvent::FocusAboutToChange: s = "QEvent::FocusAboutToChange"; break;
    case QEvent::FontChange: s = "QEvent::FontChange"; break;
    case QEvent::Gesture: s = "QEvent::Gesture"; break;
    case QEvent::GestureOverride: s = "QEvent::GestureOverride"; break;
    case QEvent::GrabKeyboard: s = "QEvent::GrabKeyboard"; break;
    case QEvent::GrabMouse: s = "QEvent::GrabMouse"; break;
    case QEvent::GraphicsSceneContextMenu: s = "QEvent::GraphicsSceneContextMenu"; break;
    case QEvent::GraphicsSceneDragEnter: s = "QEvent::GraphicsSceneDragEnter"; break;
    case QEvent::GraphicsSceneDragLeave: s = "QEvent::GraphicsSceneDragLeave"; break;
    case QEvent::GraphicsSceneDragMove: s = "QEvent::GraphicsSceneDragMove"; break;
    case QEvent::GraphicsSceneDrop: s = "QEvent::GraphicsSceneDrop"; break;
    case QEvent::GraphicsSceneHelp: s = "QEvent::GraphicsSceneHelp"; break;
    case QEvent::GraphicsSceneHoverEnter: s = "QEvent::GraphicsSceneHoverEnter"; break;
    case QEvent::GraphicsSceneHoverLeave: s = "QEvent::GraphicsSceneHoverLeave"; break;
    case QEvent::GraphicsSceneHoverMove: s = "QEvent::GraphicsSceneHoverMove"; break;
    case QEvent::GraphicsSceneMouseDoubleClick: s = "QEvent::GraphicsSceneMouseDoubleClick"; break;
    case QEvent::GraphicsSceneMouseMove: s = "QEvent::GraphicsSceneMouseMove"; break;
    case QEvent::GraphicsSceneMousePress: s = "QEvent::GraphicsSceneMousePress"; break;
    case QEvent::GraphicsSceneMouseRelease: s = "QEvent::GraphicsSceneMouseRelease"; break;
    case QEvent::GraphicsSceneMove: s = "QEvent::GraphicsSceneMove"; break;
    case QEvent::GraphicsSceneResize: s = "QEvent::GraphicsSceneResize"; break;
    case QEvent::GraphicsSceneWheel: s = "QEvent::GraphicsSceneWheel"; break;
    case QEvent::GraphicsSceneLeave: s = "QEvent::GraphicsSceneLeave"; break;
    case QEvent::Hide: s = "QEvent::Hide"; break;
    case QEvent::HideToParent: s = "QEvent::HideToParent"; break;
    case QEvent::HoverEnter: s = "QEvent::HoverEnter"; break;
    case QEvent::HoverLeave: s = "QEvent::HoverLeave"; break;
    case QEvent::HoverMove:
        return QWidget::event(ev);
        break;

    case QEvent::IconDrag: s = "QEvent::IconDrag"; break;
    case QEvent::IconTextChange: s = "QEvent::IconTextChange"; break;
    case QEvent::InputMethod: s = "QEvent::InputMethod"; break;
    case QEvent::InputMethodQuery: s = "QEvent::InputMethodQuery"; break;
    case QEvent::KeyboardLayoutChange: s = "QEvent::KeyboardLayoutChange"; break;
    case QEvent::KeyPress: s = "QEvent::KeyPress"; break;
    case QEvent::KeyRelease: s = "QEvent::KeyRelease"; break;
    case QEvent::LanguageChange: s = "QEvent::LanguageChange"; break;
    case QEvent::LayoutDirectionChange: s = "QEvent::LayoutDirectionChange"; break;
    case QEvent::LayoutRequest: s = "QEvent::LayoutRequest"; break;
    case QEvent::Leave: s = "QEvent::Leave"; break;
    case QEvent::LeaveWhatsThisMode: s = "QEvent::LeaveWhatsThisMode"; break;
    case QEvent::LocaleChange: s = "QEvent::LocaleChange"; break;
    case QEvent::NonClientAreaMouseButtonDblClick: s = "QEvent::NonClientAreaMouseButtonDblClick"; break;
    case QEvent::NonClientAreaMouseButtonPress: s = "QEvent::NonClientAreaMouseButtonPress"; break;
    case QEvent::NonClientAreaMouseButtonRelease: s = "QEvent::NonClientAreaMouseButtonRelease"; break;
    case QEvent::NonClientAreaMouseMove: s = "QEvent::NonClientAreaMouseMove"; break;
    case QEvent::MacSizeChange: s = "QEvent::MacSizeChange"; break;
    case QEvent::MetaCall: s = "QEvent::MetaCall"; break;
    case QEvent::ModifiedChange: s = "QEvent::ModifiedChange"; break;
    case QEvent::MouseButtonDblClick: s = "QEvent::MouseButtonDblClick"; break;
    case QEvent::MouseButtonPress: s = "QEvent::MouseButtonPress"; break;
    case QEvent::MouseButtonRelease: s = "QEvent::MouseButtonRelease"; break;
    case QEvent::MouseMove: s = "QEvent::MouseMove"; break;
    case QEvent::MouseTrackingChange: s = "QEvent::MouseTrackingChange"; break;
    case QEvent::Move: s = "QEvent::Move"; break;
    case QEvent::NativeGesture: s = "QEvent::NativeGesture"; break;
    case QEvent::OrientationChange: s = "QEvent::OrientationChange"; break;
    case QEvent::Paint: s = "QEvent::Paint"; break;
    case QEvent::PaletteChange: s = "QEvent::PaletteChange"; break;
    case QEvent::ParentAboutToChange: s = "QEvent::ParentAboutToChange"; break;
    case QEvent::ParentChange: s = "QEvent::ParentChange"; break;
    case QEvent::PlatformPanel: s = "QEvent::PlatformPanel"; break;
    case QEvent::PlatformSurface: s = "QEvent::PlatformSurface"; break;
    case QEvent::Polish:
        return QWidget::event(ev);
        break;

    case QEvent::PolishRequest: s = "QEvent::PolishRequest"; break;
    case QEvent::QueryWhatsThis: s = "QEvent::QueryWhatsThis"; break;
    case QEvent::Quit: s = "QEvent::Quit"; break;
    case QEvent::ReadOnlyChange: s = "QEvent::ReadOnlyChange"; break;
    case QEvent::RequestSoftwareInputPanel: s = "QEvent::RequestSoftwareInputPanel"; break;
    case QEvent::Resize: s = "QEvent::Resize"; break;
    case QEvent::ScrollPrepare: s = "QEvent::ScrollPrepare"; break;
    case QEvent::Scroll: s = "QEvent::Scroll"; break;
    case QEvent::Shortcut: s = "QEvent::Shortcut"; break;
    case QEvent::ShortcutOverride: s = "QEvent::ShortcutOverride"; break;
    case QEvent::Show:
        return QWidget::event(ev);
        break;

    case QEvent::ShowToParent: s = "QEvent::ShowToParent"; break;
    case QEvent::SockAct: s = "QEvent::SockAct"; break;
    case QEvent::StateMachineSignal: s = "QEvent::StateMachineSignal"; break;
    case QEvent::StateMachineWrapped: s = "QEvent::StateMachineWrapped"; break;
    case QEvent::StatusTip: s = "QEvent::StatusTip"; break;
    case QEvent::StyleChange: s = "QEvent::StyleChange"; break;
    case QEvent::TabletMove: s = "QEvent::TabletMove"; break;
    case QEvent::TabletPress: s = "QEvent::TabletPress"; break;
    case QEvent::TabletRelease: s = "QEvent::TabletRelease"; break;
    case QEvent::TabletEnterProximity: s = "QEvent::TabletEnterProximity"; break;
    case QEvent::TabletLeaveProximity: s = "QEvent::TabletLeaveProximity"; break;
    case QEvent::TabletTrackingChange: s = "QEvent::TabletTrackingChange"; break;
    case QEvent::ThreadChange: s = "QEvent::ThreadChange"; break;
    case QEvent::Timer: s = "QEvent::Timer"; break;
    case QEvent::ToolBarChange: s = "QEvent::ToolBarChange"; break;
    case QEvent::ToolTip: s = "QEvent::ToolTip"; break;
    case QEvent::ToolTipChange: s = "QEvent::ToolTipChange"; break;
    case QEvent::TouchBegin: s = "QEvent::TouchBegin"; break;
    case QEvent::TouchCancel: s = "QEvent::TouchCancel"; break;
    case QEvent::TouchEnd: s = "QEvent::TouchEnd"; break;
    case QEvent::TouchUpdate: s = "QEvent::TouchUpdate"; break;
    case QEvent::UngrabKeyboard: s = "QEvent::UngrabKeyboard"; break;
    case QEvent::UngrabMouse: s = "QEvent::UngrabMouse"; break;
    case QEvent::UpdateLater: s = "QEvent::UpdateLater"; break;
    case QEvent::UpdateRequest:
        initialize_mainwindow();
        return QWidget::event(ev);
        break;
    case QEvent::WhatsThis: s = "QEvent::WhatsThis"; break;
    case QEvent::WhatsThisClicked: s = "QEvent::WhatsThisClicked"; break;
    case QEvent::Wheel: s = "QEvent::Wheel"; break;
    case QEvent::WinEventAct: s = "QEvent::WinEventAct"; break;
    case QEvent::WindowActivate: s = "QEvent::WindowActivate"; break;
    case QEvent::WindowBlocked: s = "QEvent::WindowBlocked"; break;
    case QEvent::WindowDeactivate: s = "QEvent::WindowDeactivate"; break;
    case QEvent::WindowIconChange: s = "QEvent::WindowIconChange"; break;
    case QEvent::WindowStateChange: s = "QEvent::WindowStateChange"; break;
    case QEvent::WindowTitleChange: s = "QEvent::WindowTitleChange"; break;
    case QEvent::WindowUnblocked: s = "QEvent::WindowUnblocked"; break;
    case QEvent::WinIdChange: s = "QEvent::WinIdChange"; break;
    case QEvent::ZOrderChange: s = "QEvent::ZOrderChange"; break;
    default: s = "Unknown";
    }
    if (ev->type() == QEvent::Show || ev->type() == QEvent::UpdateRequest )
        qDebug() << "MainWindow::event()*******************: Type = " << s;

//////////////////////////////////






#endif
