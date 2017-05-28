// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


import QtQuick 2.8

Rectangle {
    property alias text: label.text
    property bool textAlignRight: false

    width: rpx(140)
    height: label.font.pixelSize * 1.5
    color: "#333"

    Text {
        id: label
        color: parent.activeFocus ? "#3cc" : "#eee"
        font {
            family: "Roboto"
            pixelSize: rpx(18)
        }
        anchors {
            verticalCenter: parent.verticalCenter
            left: textAlignRight ? undefined : parent.left
            right: textAlignRight ? parent.right : undefined
            leftMargin: rpx(5); rightMargin: rpx(5)
        }
    }
}