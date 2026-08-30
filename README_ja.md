# esp_extender

`esp_extender` は、ESP32に接続されたデバイスをSPIKE RTから利用するための拡張コンポーネントです。通信処理、カメラ、カラーセンサー、コマンド配送、ボタン入力、キャリブレーション関連の処理をまとめて提供します。

## 主な機能

- SPIKE/PUPデバイスインターフェースを使用したESP32通信
- コマンドの周期送信・受信処理
- センサー種別に応じたコマンドハンドラの登録・配送
- カメラからの色ID、YUV値、12点分の色データ取得
- カラーセンサーからの色ID、RGBC値、通知色、キャリブレーション要求
- ボタンの周期ポーリングと、呼び出し元ごとの短押し・長押し判定
- カメラおよびカラーセンサーのキャリブレーション用タスク

## ディレクトリ構成

```text
esp_extender/
├── include/
│   ├── lump_calib.h
│   ├── lump_camera.h
│   ├── lump_color.h
│   ├── lump_comm.h
│   ├── lump_comm_tsk.h
│   └── lump_sensors.h
├── src/
│   ├── header/
│   │   ├── lump_button_input.h
│   │   ├── lump_dispatch.h
│   │   ├── lump_receive.h
│   │   └── ...
│   └── *.c
├── include/lump_comm.cfg
├── esp_extender.cfg
└── Makefile
```

## 組み込み方法

SPIKE RTプロジェクトの一部としてビルドすることを想定しています。付属の `Makefile` では、ソースディレクトリをカーネルの検索対象に追加し、`include` をヘッダー検索パスとして登録し、各実装ファイルを `SYSSVC_COBJS` に追加しています。

`include/lump_comm.cfg` では、通信、コマンド配送、ボタン、キャリブレーション関連のタスクを生成しています。送信・受信の周期実行は10 ms間隔で設定されています。

## ヘッダーファイル

| ヘッダー | 役割 |
|---|---|
| `lump_comm.h` | ESP32デバイス初期化とコマンド送信 |
| `lump_comm_tsk.h` | タスク優先度、スタックサイズ、タスクエントリポイント |
| `lump_receive.h` | 受信コマンドキュー |
| `lump_dispatch.h` | コマンドハンドラ登録 |
| `lump_camera.h` | カメラ設定とデータ取得 |
| `lump_color.h` | カラーセンサー設定とデータ取得 |
| `lump_sensors.h` | 共通色IDとインスタンス数定義 |
| `lump_calib.h` | キャリブレーションモード制御 |
| `src/header/lump_button_input.h` | ボタン状態取得と短押し・長押し判定 |

## 設定上の注意

ESP32の接続先ポートと動作モードは `lump_comm.h` の `ESP32_PORT` と `ESP32_MODE` で設定します。コマンドのペイロード長は `LUMP_PAYLOAD_LEN` で定義されています。タスク優先度と既定のスタックサイズは `lump_comm_tsk.h` で定義されています。

プロトコルの詳細値やアプリケーション固有の動作については、付属のソースコードおよび設定ファイルを参照してください。
