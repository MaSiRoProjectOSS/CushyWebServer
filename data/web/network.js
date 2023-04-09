if (!JS_Network) {
    var JS_Network = {
        default_name_ap: "",
        default_name_sta: "",
        timeoutID: null,
        scanID: null,
        network_list: Array(),
        timerInterval: 3000,
        scanInterval: 1500,
        on_change: function () {
            if (null != JS_Network.timeoutID) {
                clearTimeout(JS_Network.timeoutID);
                JS_Network.timeoutID = null;
            }
        },
        reception_message: function (data) {
            if (null != data) {
                if ("OK" == data.result) {
                    document.getElementById("article_setting").classList.add("div_hide");
                    document.getElementById("article_message").classList.remove("div_hide");
                }
            }
        },
        set_network: function () {
            let mode = (true === document.getElementById("mode_ap").checked) ? 1 : 0;
            JS_AJAX.post("/set/network?id=" +
                document.getElementById("network_ssid").value
                + "&pa=" +
                document.getElementById("network_pass").value
                + "&ap=" + mode).then(
                    ok => JS_Network.reception_message(ok)
                    , error => console.error(error.status.messages)
                );
        },
        select_mode: function (sta) {
            JS_Network.on_change();
            var elem_list = document.getElementById("network_list");
            var elem_scan = document.getElementById("network_scan");
            var elem_ssid = document.getElementById("network_ssid");

            if (0 === sta) {
                elem_list.disabled = true;
                elem_scan.classList.add("b_disabled");
                elem_ssid.value = JS_Network.default_name_ap;
            } else {
                elem_list.disabled = false;
                elem_scan.classList.remove("b_disabled");
                elem_ssid.value = JS_Network.default_name_sta;
            }
        },
        select_list: function (id) {
            JS_Network.on_change();
            var set_name = "";
            for (var i = 0; i < JS_Network.network_list.length; i++) {
                if (id == JS_Network.network_list[i].index) {
                    set_name = JS_Network.network_list[i].name;
                    break;
                }
            }
            document.getElementById("network_ssid").value = set_name;
            JS_Network.default_name_sta = set_name;
        },
        make_option: function (index, name, power) {
            let opt = document.createElement("option");
            opt.value = index;
            opt.innerHTML = ((0 < power) ? "  [" + JS_Network.padding(power, 3, "&ensp;") + "%]  " : "") + name;
            return opt;
        },
        set_list: function (data) {
            if (null != data) {
                if ("OK" == data.result) {
                    var elm = document.getElementById("network_list");
                    JS_Network.network_list = [];
                    elm.innerHTML = "";
                    for (var i = 0; i < data.data.list.length; i++) {
                        var item = data.data.list[i];
                        JS_Network.network_list.push({ index: i, name: item.name, power: item.power });
                        let opt = document.createElement("option");
                        opt.value = i;
                        elm.appendChild(JS_Network.make_option(i, item.name, item.power));
                    }
                    text = "non-public ...";
                    var opt_public = JS_Network.make_option(JS_Network.network_list.length, JS_Network.padding(text, 7 + text.length, "&ensp;"), -1);
                    opt_public.style.color = "darkgrey";
                    elm.appendChild(opt_public);
                }
            }
        },
        padding: function (value, len, pad) {
            var buf = "" + value;
            for (var i = buf.length; i < len; i++) {
                buf = pad + buf;
            }
            return buf;
        },
        scan: function () {
            JS_AJAX.get('/make/net_list').then(
                ok => {
                    let ret = false;
                    if (null != ok) {
                        if ("OK" == ok.result) {
                            JS_Network.scanning();
                            ret = true;
                        }
                    }
                    if (false === ret) {
                        console.error("I declined your request.")
                    }
                }
                , error => console.error(error.status.messages)
            );
        },
        scanning: function () {
            JS_AJAX.get('/get/net_list').then(
                ok => {
                    let ret = false;
                    if (null != ok) {
                        if ("OK" == ok.result) {
                            if (0 <= ok.data.ret) {
                                JS_Network.set_list(ok);
                                ret = true;
                            } else if (-2 == ok.data.ret) {
                                ret = true;
                                console.error("Processing was interrupted.")
                            }
                        }
                    }
                    if (false === ret) {
                        setTimeout(JS_Network.scanning, JS_Network.scanInterval);
                    }
                }
                , error => console.error(error.status.messages)
            );
        },
        retry_begin: function (data, time_ms) {
            JS_Network.timeoutID = setTimeout(JS_Network.begin, time_ms);
        },
        set_info: function (data) {
            var result = false;
            if (null != data) {
                if ("OK" == data.result) {
                    JS_Network.default_name_ap = data.data.AP.name;
                    JS_Network.default_name_sta = data.data.STA.name;
                    var ssid_name = data.data.AP.name;
                    var mode = "mode_ap";
                    if (1 == data.data.STA.enable) {
                        mode = "mode_sta";
                        ssid_name = data.data.STA.name;
                    }
                    document.getElementById("network_ssid").value = ssid_name;
                    document.getElementById(mode).checked = true;
                    JS_Network.select_mode();
                    JS_Network.scan();
                    result = true;
                }
            }
            if (false === result) {
                JS_Network.retry_begin(data, JS_Network.timerInterval);
            }
        },
        begin: function () {
            JS_Network.on_change();
            JS_AJAX.get('/get/network').then(
                ok => JS_Network.set_info(ok)
                , error => JS_Network.retry_begin(error, JS_Network.timerInterval)
            );
        }
    }
}
window.onload = function () {
    JS_Network.begin();
};
