import QtQuick 1.1

Text {
    id: textItem
    function fetchAnimal() {
        var doc = new XMLHttpRequest();
        doc.onreadystatechange = function() {
            if (doc.readyState === XMLHttpRequest.DONE) {
                textItem.text = doc.responseText
            }
        }
        doc.open("GET", "http://animals.ivolo.me/")
        doc.send()
    }
    
    font.family: "monospace"
    
    Behavior on x { NumberAnimation { duration: 1000; easing.type: Easing.InQuad } }
    Behavior on y { NumberAnimation { duration: 1000; easing.type: Easing.InQuad } }
}