async function WaitForServerToBeAccessible(timeOut) {
    var promise = new Promise((resolve, reject) => {
        var successCount = 0;
        var startTime = Date.now();
        var timerId = setInterval(async function () {
            var acc = await IsWebServerAccessible();
            if (acc) {
                if (++successCount == 1) {
                    clearInterval(timerId);
                    resolve(true);
                }
            } else {
                successCount = 0;
                if (Date.now() - startTime > timeOut) {
                    clearInterval(timerId);
                    resolve(false);
                }
            }
        }, 5500);
    });

    return await promise;
}

async function IsWebServerAccessible() {
    promise = new Promise((resolve, reject) => {
        var req = new XMLHttpRequest();

        req.timeout = 5000; // it could happen that the request takes a very long time

        req.onreadystatechange = function () {
            if (req.readyState == 4) {
                resolve(200 <= req.status && req.status < 300);
            }
        }

        req.ontimeout = function () {
            resolve(false);
        }

        req.open("GET", appBase + "dummy", true);
        req.setRequestHeader("Cache-Control", "no-cache")
        req.send();
    });

    return await promise;
}
