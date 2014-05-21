WorkerScript.onMessage = function(message) {
    // ... long-running operations and calculations are done here
    WorkerScript.sendMessage({ 'reply': 'Mouse is at ' + message.x + ',' + message.y })
}
