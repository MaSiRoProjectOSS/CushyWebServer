if (!JS_NW) {
    var JS_NW = {
        info: {
            AP: {
                default: "",
                ip: "",
                enable: 0
            },
            STA: {
                default: "",
                ip: "",
                enable: 0,
                list: []
            }
        },
        timeoutID: null,
        network_list: Array(),
        timerInterval: 3000,
        scanInterval: 1500,

        _set_class(name, flag) {
            var elem = document.getElementById(name);
            if (true === flag) {
                elem.classList.remove("b_disabled");
                elem.disabled = !flag;
                elem.readOnly = !flag;
            } else {
                elem.classList.add("b_disabled");
                elem.disabled = !flag;
                elem.readOnly = !flag;
            }
            return elem;
        },
        _reception_message: function (res) {
            if (null != res) {
                if ("OK" == res.result) {
                    document.getElementById("article_setting").classList.add("div_hide");
                    document.getElementById("article_message").classList.remove("div_hide");
                }
            }
        },
        _make_option: function (index, name, power) {
            let opt = document.createElement("option");
            opt.value = index;
            opt.innerHTML = ((0 < power) ? "  [" + JS_NW._padding(power, 3, "&ensp;") + "%]  " : "") + name;
            return opt;
        },
        _padding: function (value, len, pad) {
            var buf = "" + value;
            for (var i = buf.length; i < len; i++) {
                buf = pad + buf;
            }
            return buf;
        },
        _set_list: function (res) {
            if (null != res) {
                if ("OK" == res.result) {
                    var elm = document.getElementById("network_list");
                    JS_NW.network_list = [];
                    elm.innerHTML = "";
                    for (var i = 0; i < res.data.list.length; i++) {
                        var item = res.data.list[i];
                        JS_NW.network_list.push({ index: i, name: item.name, power: item.power });
                        let opt = document.createElement("option");
                        opt.value = i;
                        elm.appendChild(JS_NW._make_option(i, item.name, item.power));
                    }
                    text = "non-public ...";
                    var opt_public = JS_NW._make_option(JS_NW.network_list.length, JS_NW._padding(text, 7 + text.length, "&ensp;"), -1);
                    opt_public.style.color = "darkgrey";
                    elm.appendChild(opt_public);
                }
            }
        },
        _scanning: function () {
            JS_AJAX.get('/CushyWebServer/list_get').then(
                ok => {
                    let ret = false;
                    if (null != ok) {
                        if ("OK" == ok.result) {
                            if (0 <= ok.data.ret) {
                                JS_NW._set_list(ok);
                                ret = true;
                            } else if (-2 == ok.data.ret) {
                                ret = true;
                                console.error("Processing was interrupted.")
                            }
                        }
                    }
                    if (false === ret) {
                        setTimeout(JS_NW._scanning, JS_NW.scanInterval);
                    }
                }
                , error => console.error("error.status.messages")
            );
        },
        _scan: function () {
            JS_AJAX.get('/CushyWebServer/list_make').then(
                ok => {
                    let ret = false;
                    if (null != ok) {
                        if ("OK" == ok.result) {
                            JS_NW._scanning();
                            ret = true;
                        }
                    }
                    if (false === ret) {
                        console.error("I declined your request.")
                    }
                }
                , error => console.error("error.status.messages")
            );
        },
        _checked_elm: function (id) {
            document.getElementById(id).checked = true;
        },
        _replace_sta_order: function (id, data) {
            var text = "";
            for (var i = 0; i < data.length; i++) {
                var it = document.createElement('input');
                var lb = document.createElement('span');
                var li = document.createElement('li');
                it.title = "-";
                it.placeholder = "0";
                it.type = "radio";
                it.name = "sta_select";
                it.value = i;
                lb.onclick = JS_NW.select_sta_mode(i);
                lb.setAttribute('onclick', "JS_NW.select_sta_mode(" + i + ")");
                lb.innerHTML = "[" + i + "] " + data[i];

                li.appendChild(it);
                li.appendChild(lb);
                text += li.outerHTML;
            }
            document.getElementById(id).innerHTML = text;
        },
        _set_info: function (res) {
            var result = false;
            if (null != res) {
                if ("OK" == res.result) {
                    JS_NW.info = res.data;
                    JS_NW.info.AP.default = JS_NW.info.AP.ssid;
                    JS_NW.info.STA.default = JS_NW.info.STA.ssid;

                    document.getElementById("connect_ap").innerText = JS_NW.info.AP.ssid;
                    document.getElementById("connect_sta").innerText = JS_NW.info.STA.ssid;
                    document.getElementById("ip_ap").innerText = JS_NW.info.AP.ip;
                    document.getElementById("ip_sta").innerText = JS_NW.info.STA.ip;

                    JS_NW._checked_elm((0 == JS_NW.info.AP.enable) ? "disable_ap" : "enable_ap");
                    JS_NW._checked_elm((0 == JS_NW.info.STA.enable) ? "disable_sta" : "enable_sta");
                    JS_NW.check_mode_ap(JS_NW.info.AP.enable);
                    JS_NW.check_mode_sta(JS_NW.info.STA.enable);
                    JS_NW._replace_sta_order("radio_sta", JS_NW.info.STA.list);
                    JS_NW.select_sta_mode(JS_NW.info.STA.selected);
                    JS_NW._scan();
                    result = true;
                }
            }
            if (false === result) {
                JS_NW._retry_begin();
            }
        },
        _retry_begin: function () {
            JS_NW.timeoutID = setTimeout(JS_NW.begin, JS_NW.timerInterval);
        },

        check_mode: function (id, mode) {
            JS_NW.on_change();

            var flag = true;
            if (0 === mode) {
                flag = false;
            } else {
                flag = true;
            }
            var elem_ssid = JS_NW._set_class("ssid_" + id, flag);
            elem_ssid.value = (id == "ap") ? JS_NW.info.AP.default : JS_NW.info.STA.default;
            if (id == "ap") {
                document.getElementById("hostname_" + id).value = JS_NW.info.AP.hostname;
            }
            JS_NW._set_class("pass_" + id, flag);
            JS_NW._set_class("hostname_" + id, flag);
            document.getElementById("enable_" + id).checked = flag;
            document.getElementById("disable_" + id).checked = !flag;
            return flag;
        },
        check_mode_ap: function (mode) {
            JS_NW.check_mode('ap', mode);
        },
        check_mode_sta: function (mode) {
            var flag = JS_NW.check_mode("sta", mode);
            JS_NW._set_class("selector_sta", flag);
            JS_NW._set_class("scan_sta", flag);
            JS_NW._set_class("network_list", flag);
            var elem = document.getElementById("selector_sta");
            if (true === flag) {
                elem.classList.remove("div_hide");
            } else {
                elem.classList.add("div_hide");
            }
        },
        set_network_ap: function (id) {
            JS_NW.set_network('ap', id);
        },
        set_network_sta: function (id) {
            JS_NW.set_network('sta', id);
        },
        set_network: function (id) {
            let mode = (id == "ap") ? 1 : 0;
            let num = (id == "ap") ? 0 : document.getElementById("radio_sta").elements["sta_select"].value;
            let state = document.getElementById("enable_" + id).checked ? 1 : 0;

            JS_AJAX.post("/CushyWebServer/set?id=" +
                document.getElementById("ssid_" + id).value
                + "&pa=" +
                document.getElementById("pass_" + id).value
                + "&hostname=" +
                document.getElementById("hostname_" + id).value
                + "&ap=" + mode + "&num=" + num + "&state=" + state).then(
                    ok => JS_NW._reception_message(ok)
                    , error => console.error("error.status.messages")
                );
        },
        select_list: function (id) {
            JS_NW.on_change();
            var set_name = "";
            for (var i = 0; i < JS_NW.network_list.length; i++) {
                if (id == JS_NW.network_list[i].index) {
                    set_name = JS_NW.network_list[i].name;
                    break;
                }
            }
            document.getElementById("ssid_sta").value = set_name;
            JS_NW.info.STA.default = set_name;
        },
        select_sta_mode: function (num) {
            var elm_sel = document.getElementsByName('sta_select');
            if (0 <= num) {
                if (num < elm_sel.length) {
                    elm_sel.item(num).checked = true;
                    document.getElementById('ssid_sta').value = JS_NW.info.STA.list[num];
                    document.getElementById('hostname_sta').value = JS_NW.info.STA.hostname[num];
                }
            }
        },
        on_change: function () {
            if (null != JS_NW.timeoutID) {
                clearTimeout(JS_NW.timeoutID);
                JS_NW.timeoutID = null;
            }
        },
        begin: function () {
            JS_NW.on_change();
            JS_AJAX.get('/CushyWebServer/get').then(
                ok => JS_NW._set_info(ok)
                , error => JS_NW._retry_begin()
            );
        }
    }
}
window.onload = function () {
    JS_NW.begin();
};
