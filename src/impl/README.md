# web viewer

## クラス構成について

以下のクラスによって構成されている。

| クラス名                      | 機能                                                                                                                            |
| ----------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| WebViewer                     | 継承元のWebページ等を管理しているクラス<br>このクラスでThreadなども管理している                                                 |
| L WebCommunication            | Webページ表示のためのコンテキストやデフォルトページ（Wifi接続先変更ページ"http://xxx.xxx.xxx.xxx/network"）を管理しているクラス |
| &ensp; L WebManagerConnection | Networkの接続を管理しているクラス                                                                                               |
| &ensp; L WebManagerSetting    | 接続情報をSPIFFSで管理するクラス                                                                                                |


クラス「WebViewer」を継承することで"WebServer"を操作することが出来、
自由にページを追加する事ができる。

```c++
#include <web_viewer.hpp>

class WebViewerCustom : public MaSiRoProject::Web::WebViewer {
public:
    WebViewerCustom();

protected:
    bool setup_server(WebServer *server) override;

private:
   void handle_root();
};

bool WebViewerCustom::setup_server(WebServer *server){
  server->on("/", std::bind(&WebViewerCustom::handle_root, this, std::placeholders::_1));
}

```


## 設定について

下記がデフォルトの設定を行っている。



"web_default.hpp"は汎用性にかけるのでAPIの見直しを行う。



## その他

ESP32の<WebServer.h>はエラー時にログを出力している。
CORE_DEBUG_LEVELで制御可能

```ini
[env]
build_flags =
	-DCORE_DEBUG_LEVEL=-1
```
