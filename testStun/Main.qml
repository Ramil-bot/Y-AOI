import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic


ApplicationWindow {
    id: window
    width: 640
    height: 480
    minimumWidth: 200
    minimumHeight: 250
    visible: true
    title: qsTr("Hello World")
    
    TextField {
      id: editor
      anchors.centerIn: parent
      width: 120
      height: 20
    }
}
