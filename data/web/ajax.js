if (!JS_AJAX) {
    var JS_AJAX = {
        post: function (url) { return JS_AJAX.send("post", url); },
        get: function (url) { return JS_AJAX.send("get", url); },
        send: function (method, url) {
            return new Promise((resolve, reject) => {
                if ("post" != method) {
                    if ("get" != method) {
                        method = get;
                    }
                }
                const request = new XMLHttpRequest();
                request.ontimeout = () => {
                    reject({
                        subject: url,
                        result: 'NG', status: { num: 503, messages: 'Timeout' }
                    });
                };
                request.onload = function (event) {
                    if (request.readyState === 4 && request.status === 200) {
                        if (!request.responseText) {
                            reject({
                                subject: url,
                                result: 'NG', status: { num: 500, messages: 'Internal Server Error' }
                            });
                        }
                        resolve(JSON.parse(request.responseText));
                    } else {
                        reject({
                            subject: url,
                            result: 'NG', status: { num: request.status, messages: request.statusText }
                        });
                    }
                };
                request.onerror = function (event) {
                    reject({
                        subject: url,
                        result: 'NG', status: { num: 404, messages: 'Not found' }
                    });
                }
                request.timeout = 10000;
                request.open(method, url, true);
                request.send(null);
            })
        }
    }
}
