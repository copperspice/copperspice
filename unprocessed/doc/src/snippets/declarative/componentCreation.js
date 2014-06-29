//![vars]
var component;
var sprite;
//![vars]

//![func]
function createSpriteObjects() {
//![func]

//![remote]
    component = Qt.createComponent("Sprite.qml");
    if (component.status == Component.Ready)
        finishCreation();
    else
        component.statusChanged.connect(finishCreation);
//![remote]

//![local]
    component = Qt.createComponent("Sprite.qml");
    sprite = component.createObject(appWindow, {"x": 100, "y": 100});

    if (sprite == null) {
        // Error Handling
        console.log("Error creating object");
    }
//![local]

//![func-end]
}
//![func-end]

//![finishCreation]
function finishCreation() {
    if (component.status == Component.Ready) {
        sprite = component.createObject(appWindow, {"x": 100, "y": 100});
        if (sprite == null) {
            // Error Handling
            console.log("Error creating object");
        }
    } else if (component.status == Component.Error) {
        // Error Handling
        console.log("Error loading component:", component.errorString());
    }
}
//![finishCreation]
