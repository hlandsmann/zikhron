import QtQuick 2.14
Item {
    function readValues(anArray, anObject) {
        for (var i=0; i<anArray.length; i++)
            console.log("Array item:", anArray[i])

        for (var prop in anObject) {
            console.log("Object item:", prop, "=", anObject[prop])
        }
    }
}
