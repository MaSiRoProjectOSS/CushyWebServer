if (!JS_AJAX) {
    var JS_AJAX = {
        result_ip: "127.0.0.1",
        result_data: {
            AP: {
                ssid: "CushyWebServer",
                ip: "127.0.0.1",
                hostname: "ap_mode",
                enable: 1
            },
            STA: {
                ssid: "net_001_-60",
                ip: "127.0.0.1",
                enable: 1,
                selected: 0,
                list: [
                    "net_001_-60",
                    "net_002_-67",
                    "",
                    "",
                    ""
                ],
                hostname: [
                    "sta_mode_00",
                    "sta_mode_01",
                    "sta_mode_02",
                    "sta_mode_03",
                    "sta_mode_04"
                ]
            }
        },
        reload: function () {
            // location.reload();
            document.getElementById("article_setting").classList.remove("div_hide");
            document.getElementById("article_message").classList.add("div_hide");
            JS_NW.begin();
        },
        post: function (url) { return JS_AJAX.send("post", url); },
        get: function (url) { return JS_AJAX.send("get", url); },
        send: function (method, url) {
            console.log("JS_AJAX.send(" + method + ", " + url + ")");
            return new Promise((resolve, reject) => {
                if (0 <= url.indexOf("/CushyWebServer/list_get")) {
                    resolve({
                        result: 'OK',
                        status: {
                            messages: "",
                            KEY: 977
                        },
                        data: {
                            list: [
                                {
                                    name: "net_001_-60",
                                    power: -60
                                },
                                {
                                    name: "net_002_-67",
                                    power: -67
                                },
                                {
                                    name: "net_003_-68",
                                    power: -68
                                },
                                {
                                    name: "net_004_-74",
                                    power: -74
                                },
                                {
                                    name: "net_005_-81",
                                    power: -81
                                },
                                {
                                    name: "net_006_-83",
                                    power: -83
                                },
                                {
                                    name: "net_007_-84",
                                    power: -84
                                },
                                {
                                    name: "net_008_-84",
                                    power: -84
                                },
                                {
                                    name: "net_009_-85",
                                    power: -85
                                },
                                {
                                    name: "net_010_-86",
                                    power: -86
                                },
                                {
                                    name: "net_011_-88",
                                    power: -88
                                },
                                {
                                    name: "net_012_-92",
                                    power: -92
                                }
                            ],
                            ret: 13
                        }
                    });
                }
                else if (0 <= url.indexOf("/CushyWebServer/get")) {
                    resolve({
                        result: "OK",
                        status: {
                            messages: "",
                            KEY: 977
                        },
                        data: JS_AJAX.result_data
                    });
                }
                else if (0 <= url.indexOf("/CushyWebServer/list_make")) {
                    resolve({
                        result: "OK",
                        status: {
                            messages: "",
                            KEY: 977
                        },
                        data: ""
                    });
                }
                else if (0 <= url.indexOf("/CushyWebServer/set")) {
                    var txt_id = url.substring(url.indexOf("id=") + 3, url.indexOf("&pa="));
                    let hostname = url.substring(url.indexOf("&hostname=") + 10, url.indexOf("&ap="));
                    let ap_mode = Number(url.substring(url.indexOf("&ap=") + 4, url.indexOf("&num=")));
                    let num = Number(url.substring(url.indexOf("&num=") + 5, url.indexOf("&state=")));
                    let state = Number(url.substring(url.indexOf("&state=") + 7));
                    if ("" == txt_id) {
                        state = 0;
                    }
                    if (1 == ap_mode) {
                        // AP mode
                        JS_AJAX.result_data.AP.enable = state;
                        if (1 == state) {
                            JS_AJAX.result_data.AP.ssid = txt_id;
                            JS_AJAX.result_data.AP.ip = JS_AJAX.result_ip;
                            JS_AJAX.result_data.AP.hostname = hostname;
                        } else {
                            JS_AJAX.result_data.AP.ssid = "";
                            JS_AJAX.result_data.AP.ip = "";
                            JS_AJAX.result_data.AP.hostname = "";
                        }
                    } else {
                        // STA mode
                        JS_AJAX.result_data.STA.enable = state;
                        if (1 == state) {
                            JS_AJAX.result_data.STA.ssid = txt_id;
                            JS_AJAX.result_data.STA.selected = num;
                            JS_AJAX.result_data.STA.ip = JS_AJAX.result_ip;
                            JS_AJAX.result_data.STA.hostname[num] = hostname;
                            JS_AJAX.result_data.STA.list[num] = txt_id;
                        } else {
                            JS_AJAX.result_data.STA.ssid = "";
                            JS_AJAX.result_data.STA.ip = "";
                            JS_AJAX.result_data.STA.hostname[num] = "";
                            JS_AJAX.result_data.STA.list[num] = "";
                        }
                    }

                    resolve({
                        result: "OK",
                        status: {
                            messages: "",
                            KEY: 977
                        },
                        data: JS_AJAX.result_data
                    });
                }
                else {
                    reject({
                        subject: url,
                        result: 'NG', status: { num: 503, messages: 'Timeout' }
                    });
                }
            })
        }
    }
}
