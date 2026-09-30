# 移植ノウハウ＆トラブルシューティング集

e² studio と FSP (Flexible Software Package) を用いて、ルネサス公式の FreeRTOS ベース AI サンプルを μT-Kernel 3.0 (mtk3_bsp2) へ移植した際に得られた知見と罠の記録です。
同じ構成（EK-RA8P1 + μT-Kernel 3.0 + FSP 6.6.0）で開発する方の参考になるよう公開しています。

> 本文中の `make_armgcc13_junction.bat` / `build_mtk3_headless.bat` / `pfs.gdb` は開発者のローカル補助スクリプトで、リポジトリには含まれていません。

## 1. FSPコンフィギュレータの罠と回避策

### ⚠️ 【超重要】XMLファイルの直接編集とキャッシュ増殖バグ
*   **現象**: e² studio の FSPコンフィギュレータ画面を「開いたまま」、外部から `configuration.xml` を直接書き換えてエディタをリロードすると、**メモリ上の古い設定と新しいXML設定がマージされ、画面上に同じ名前の部品が2つ増殖する**致命的なバグがある。（「Unique name required」エラー）
*   **対策**: `configuration.xml` を直接書き換える場合は、**必ず e² studio を終了させてから** 行う。
*   **リカバリー**: 増殖バグが発生した場合は、e² studioを保存せずに終了し、プロジェクト内の `ra_gen` フォルダの中身をすべて削除してから再起動する。

### ⚠️ `.cproject` の直接編集は「e² studio を閉じて、バックアップを取ってから」
*   **現象**: e² studio を開いたまま `.cproject` を書き換えると、内部構造が壊れ `Generate Project Content` 時に `projDes is null` エラーになることがある。
*   **対策**: 直接編集するなら **e² studio を終了してから**、`.cproject` をバックアップし、XML として正しいか確認する（`python3 -c "import xml.dom.minidom as m;m.parse('.cproject')"`）。この方法で、C++ のインクルードパス・シンボル・リンカ設定を Debug / Release の両方へ安全に追加できた。
*   ヘッダを `src/` 直下にコピーしてフラットに置く方法は非推奨。後述の `src/bits` 事故（11章）のように、システムヘッダを上書きしてしまう危険がある。

### ⚠️ FSPモジュールの強引なXML追記は無謀
*   **現象**: FSPコンフィギュレータのXMLに対して、外部スクリプトで無理やり `<instance>` 等を挿入しても、依存関係やピン割り当てが絡み合っているため正常に生成されない。
*   **対策**: FSPコンポーネント（Stacks）を追加する場合は、必ずGUIの「New Stack」から行うこと。

## 2. ピン設定 (Pins) は「元サンプルとの差分比較」が必須
*   **教訓（訂正）**: 公式 BSP プロジェクトのピン設定は「ボード用の汎用設定」であり、カメラや LCD を動かすための設定がすべて入っているわけではない。今回は次のピンが元サンプルと違っており、それぞれ動かない原因になった。
    *   **P709 (CAM_RST)**: ピン名が `CAMERA_RESET` だったため、`CAM_RST` が定義されず、`rtos_to_mtk.h` の誤った代替定義（P309 = Ethernet 用ピン）が使われていた。
    *   **P108 (MIPI_IF_EN)**: 元サンプルは GPIO 出力、BSP では無効。無効のままだと MIPI インターフェースが有効にならず、**カメラの初期化は成功するのに画像が 1 枚も届かない**。
    *   **P606 (LCD リセット) など**: 元サンプルは LCD 用のピンを別のピン設定（`g_bsp_pin_cfg_glcd`）にまとめ、起動時に 100ms 待ってから適用している。
*   **対策**: 元サンプルの `ra_cfg.txt`（ピン一覧のテキスト）と自プロジェクトの `ra_cfg.txt` を突き合わせ、差分を 1 つずつ確認する。ピン名（Symbolic Name）はコードの `#define` と一致させる。
*   **重要**: ピンの設定変更は、`hal_data.c` ではなく `ra_gen/pin_data.c` と `ra_cfg/fsp_cfg/bsp/bsp_pin_cfg.h` に出力される。Generate 後にこの 2 つを確認する。

## 3. モジュールのリネームと μT-Kernel 側の修正
*   **対策**: FSPコンフィギュレータで部品名（`g_i2c_master0` -> `g_cam_i2c_master`）を変更した場合は、必ず `mtk3_bsp2/sysdepend/ra_fsp/devinit.c` などの μT-Kernel 側初期化コード内の変数名もセットで置換すること。

## 4. BareMetal 環境での D/AVE 2D ヒープメモリ設定
*   **対策**: μT-Kernel（BareMetal扱い）で描画エンジンを使う場合、システムのヒープメモリを要求されるため、`configuration.xml` の BSP設定で **`Heap size` を `0` から `0x1000` などに増やす**こと。

## 5. FreeRTOS から μT-Kernel への API 置換ノウハウ
1.  **互換レイヤー（特効薬）の作成**: `BaseType_t` や `vTaskDelay` 等を一括で吸収・置換する **`rtos_to_mtk.h` を作成し、全ソースで include させるのが最強のアプローチ。**
2.  **ダミーヘッダの作成**: 移植元で自動生成されていた `Threads` 関連ヘッダが存在しないことによるエラーは、空のダミーヘッダを作ってコンパイラを騙す。

## 6. AIサンプルのOSPIメモリ依存からの脱却 (ROM化)
*   **対策**: YOLO-fastest のような小規模モデルであれば、外部OSPIメモリは不要。OSPI関連の初期化コード（`ospi_b_ep.c`）は丸ごと無効化し、AIモデルは内蔵フラッシュのC配列として直接リンクさせることで移植が劇的に楽になる。

## 7. FSPバージョンアップに伴うNPU (Ethos-U) 仕様変更と多重定義
*   **現象**: FSP 5.x 向けに作られた古いAIサンプルを FSP 6.6.0 に移植すると、`ethosu_flush_dcache` などで型の不一致や `multiple definition` エラーが発生する。
*   **原因**: FSP 6.6.0 では、キャッシュ操作などの NPU ドライバ関数が `rm_ethosu.c` 側で標準実装される仕様に変わったため。
*   **対策**: FSP 6.6.0 を使用する場合、古いAIサンプル内の `ethosu_dcache.c` は不要となるため丸ごと無効化する。

## 8. Cソース側とFSP設定側のコールバック関数名のズレ
*   **対策**: リンク時に `undefined reference to 'cam_i2c_master_callback'` エラーが出た場合は、FSP GUI の Properties で指定したコールバック名と、実際のCソース内の関数名（`g_cam_i2c_master_user_callback`等）が一致しているか確認し、手動で置換する。


## 9. リンカの最適化によるエラーの隠蔽と顕在化
*   **現象**: 不要なモジュール（UART通信やボタン割り込み等）を削除してビルドが成功していたのに、OSの大元（app_main.c等）から移植先のタスクを呼び出した瞬間に、大量の undefined reference エラーが発生する。
*   **原因**: GCCのリンカ最適化（--gc-sections）により、エントリ関数がどこからも呼ばれていない状態では「使われていない不要なコード」として丸ごと削除されるため、内部の未定義エラーが隠蔽されていた。メイン処理から呼び出す（スイッチを入れる）ことで初めて真のリンクエラーが顕在化する。
*   **対策**: 削除した不要モジュール（console_output_init や print_to_console など）に対しては、空のダミー関数（スタブ）を適当なCファイル内に定義してリンクエラーを黙らせる。

## 10. C言語プロジェクトへの C++ コードの混入 (AI推論)
*   **現象**: AIモデルの出力結果を処理するコード（YOLOのバウンディングボックス計算など）が C++ (.cc や .cpp) で書かれている場合、ベースとなる μT-Kernel プロジェクトが「C言語専用プロジェクト (C Project)」として作成されていると、コンパイル時に無視されリンクエラー（face_detection が見つからない等）になる。
*   **対策**: ファイルの拡張子が .cc の場合は .cpp に変更し、e² studio のプロジェクト・プロパティから C++ ネイチャーを追加（C++ コンパイラを有効化）してビルド対象に含める必要がある。


## 11. C++ の STL ヘッダ（bits/c++config.h 等）が見つからないエラー
*   **現象**: C プロジェクトを「C/C++ プロジェクトへの変換」した後、`<cstddef>` や `<forward_list>` の内部で `bits/c++config.h: No such file or directory` が大量に出る。
*   **本当の原因（訂正）**: インクルードパスの設定漏れではなく、**Windows のパス長制限（260 文字）**。e² studio 付属のツールチェーンはパスが深く、gcc が内部で `bin/../lib/gcc/arm-none-eabi/13.2.1/../../../../` のような長いパスを作るため、深い階層のヘッダだけ開けなくなる。見分け方：`cstddef` は見つかるのに `bits/stl_iterator_base_types.h` は見つからない（ファイルの深さで成否が分かれる）。
*   **対策**:
    1.  ジャンクションで短いパスを作る：`mklink /J C:\armgcc13 "<長いツールチェーンのパス>"`（`make_armgcc13_junction.bat`）。
    2.  e² studio の「ヘルプ → Renesas ツールチェーンの追加」で `C:\armgcc13` を登録し、元の長いパス側のチェックを外す（同じバージョン名が 2 つあると区別できないため）。
*   **やってはいけないこと**: libstdc++ のヘッダを `src/bits` などにコピーして回避すること。`src` はインクルードパスに入っているため、本物のヘッダより先に読まれ、ビルド環境を壊す。
*   **C++ を足した後に必要な設定**（C 側から自動では引き継がれない）:
    *   C++ コンパイラのインクルードパスとシンボルを C コンパイラと揃える。
    *   C++ リンカにも、C リンカと同じリンカスクリプト（`fsp.ld`, `mtkernel.ld`）と `-L script` を指定する。
    *   `--specs=nosys.specs` を追加する（libstdc++ が参照する `_exit` / `_kill` / `_getpid` の未定義エラー対策。「not implemented」警告は無害）。
    *   TFLM の型だけが必要な場合は、`tensorflow/lite/core/c/common.h` など 5 本のヘッダを元サンプルからコピーし、`TF_LITE_STATIC_MEMORY` を定義する。
    *   CMSIS-NN の MVE インラインアセンブラは `-O0` だと `'asm' operand has impossible constraints` になる → `ra/arm` フォルダだけ `-O2` でコンパイルする。
*   **便利**: e² studio を閉じた状態なら、`build_mtk3_headless.bat`（`e2studioc.exe` のヘッドレスビルド）でビルドログをファイルに出せる。

## 12. カメラ（OV5640 / MIPI CSI）が動かないときの確認順
1.  **I2C で Product ID が読めない**（`camera_init` で `FSP_ERR_HW_LOCKED`）→ カメラ本体がリセット中か、クロック（XCLK）が出ていない。
    *   `CAM_RST` が正しいピン（P709）か。
    *   `g_cam_clk` が元サンプルと同じか：GPT **ch12**（P501 = GTIOC12A）、24000 kHz、**GTIOCA 出力 = 有効**、GPT 共通設定の **Pin Output Support = 有効**。
    *   GPT のクロック源：PLL2P/2（300MHz）では 24MHz ちょうどが作れない → **GPTCLK を PLL2R/2（240MHz）**にする。
    *   I2C の速度は Fast（400kHz）。OV5640 は Fast-mode Plus（1MHz）非対応。
    *   `FSP_ERR_HW_LOCKED` は I2C ドライバのエラーではなく、`camera_init()` が「ID 不一致」のときに返す値。
2.  **初期化は成功するが画像が来ない**（VIN 割り込みが 1 回も発生しない）
    *   **P108 (MIPI_IF_EN)** が GPIO 出力になっているか（これが最終原因だった）。
    *   BSP の **SDRAM Support = 有効**（VIN の受信バッファやフレームバッファは `.sdram_noinit` に置かれる）。
    *   MIPI PHY のタイミング値を元サンプルに合わせる。
3.  **I2C 完了待ちのコードの注意**: 転送を開始した後に完了フラグをクリアすると、割り込みが速いと完了を見逃す。また「65535 回ループ」のタイムアウトは 1GHz の CM85 では 1ms 未満。時間ベースのタイムアウトにする。

## 13. LCD（パラレル・グラフィックス拡張ボード 7 インチ 1024×600）が映らないときの確認
*   BSP の GLCDC 設定は 480×854 になっていたが、元サンプルは **1024×600**（H: total 1334 / back porch 300、V: total 635 / back porch 30、同期エッジ = 立ち上がり、コールバック `lcd_glcdc_callback`）。
*   元サンプルは LCD 用のピン（P606 = LCD リセットを High、データ線、P514 = バックライト）を別のピン設定にまとめ、起動時に 100ms 待ってから適用している（パネルのパワーオンリセット対策）。μT-Kernel 版では `hal_warmstart.c` の `R_IOPORT_Open` の後で同じ処理を行う。
*   **このボードの LCD はパラレル RGB 接続**（GLCDC → ピンヘッダ J1 → グラフィックス拡張ボード）。サンプルのファイル名にある `mipi` はカメラ側のことなので混同しない。
*   ソフト側を全部確認しても映らないときは、17〜20 章の手順でハード故障を切り分ける。2026-09-22 時点の貸与機は、工場出荷時デモでも映らずハード故障と判断した。

## 14. μT-Kernel と FSP の組み合わせで確認済みのこと
*   mtk3_bsp2 は起動時に FSP のベクタテーブルを RAM へコピーして使うため、FSP の割り込みハンドラはそのまま呼ばれる。FSP の IRQ に対して `tk_def_int()` を呼んではいけない。
*   割り込み優先度 0 は使わない（カーネルの最高外部割り込みレベル `INTPRI_MAX_EXTINT_PRI` が 1 のため、0 はカーネル管理外になる）。
*   FreeRTOS の EventGroup は、`tk_wai_flg`（`TWF_BITCLR` で「待ったビットだけクリア」）、`tk_clr_flg`（引数は AND パターンなので `~bits` を渡す）、`tk_ref_flg` で置き換えられる。**`xEventGroupWaitBits` を「何もしないスタブ」のままにすると、タスクが待たずに空回りする。**
*   μT-Kernel のタスク内で待つときは `R_BSP_SoftwareDelay`（ビジーウェイト）ではなく `tk_dly_tsk` を使う。

## 15. 動かないときの最強の切り分け方法：元サンプルとのレジスタ比較
*   元サンプル（FreeRTOS 版）と自プロジェクトを同じボードで順番にデバッグ実行し、gdb で周辺機能のレジスタをダンプして比較する。
    *   Debugger Console で `set logging file <パス>` → `set logging enabled on` → `x/64wx <アドレス>` → `set logging enabled off`。
    *   主なアドレス：MIPI PHY `0x40346C00`、MIPI CSI `0x40347000`、VIN `0x40347400`、PORT `0x40400000`〜。
    *   `call rdSensorReg16_8(0x300A, (unsigned char*)<空きRAM>)` のように、カメラのレジスタも gdb から読める。
*   今回は「カメラ側レジスタは同一、ポートの出力値だけが違う」という比較結果から P108 にたどり着いた。
*   元サンプルの `configuration.xml` はモジュール単位・ピン単位で Python から差分を取れる。GUI で 1 項目ずつ見比べるより速く確実。

## 16. e² studio 操作の注意（リモート／自動操作時）
*   e² studio のタイトルバーやタブをダブルクリックすると、e² studio が終了したり PC がロックされたりすることがあった。ダブルクリックは避け、メニューやショートカット（Ctrl+M で最大化など）を使う。

## 17. LCD が映らない原因を「ソフト or ハード」で切り分ける手順（2026-09-22 実施）
公式サンプル `ek_ra8p1_vision_face_detection_yolo_fastest_FSP660` で「バックライトは点くが画面は真っ黒／真っ白」という症状を、以下の順で切り分けた。結論はハードウェア（LCD パネルまたは拡張ボード）の故障。

1.  **計測用変数（プローブ）で「どこまで動いているか」を可視化する**
    *   `src/common_util.c` に `volatile uint32_t dbg_*` を追加し、カメラ VIN コールバック、GLCDC コールバック、各スレッドのループ回数・進行段階を記録する。
    *   デバッグ中に Debugger Console で値を読む。`xEventGroupGetBits` はマクロなので gdb から呼べない。**イベントグループは `p/x *(unsigned int*)g_ai_app_event` で読む**（先頭メンバが `uxEventBits`）。
    *   実測値：イベント 0x140F、カメラフレーム 約30fps、GLCDC 割り込み 約35回/秒、AI 推論 約20回/秒 → **ソフトは正常に回っている**。
2.  **フレームバッファの中身を直接見る**
    *   `x/4hx &fb_background[0][0]` でカラーバーのデータが SDRAM に正しく入っていることを確認（`fb_background` は `.sdram_noinit`、アドレス 0x681C2000 付近）。
    *   `p g_lcd_glcdc_ctrl` の `state` が `DISPLAY_STATE_DISPLAYING` であることを確認。
3.  **カラーバーテスト**：`display_layer.c` の `display_image_buffer_initialize()` で黒塗り `memset` の `#if 1` を `#if 0`、カラーバーの `#if 0` を `#if 1` にし、`camera_display_thread_entry.c` の `do_face_reconition_screen()` をコメントアウトする。
4.  **ピン設定（PFS）をランタイムで読む**：PFS は `0x40400800 + ポート*0x40 + ピン*4`。LCD ピンの期待値は `19010400`（PSEL=0x19 = LCD_GRAPHICS、PMR=1、drive mid）、クロック P515 は `19010c00`。**末尾の bit1 は PIDR（その瞬間のピンの入力レベル）なので 2 が立っていても正常**。
    *   まとめて読む gdb スクリプトを作って `source <パス>` で実行すると速い（`TRON/pfs.gdb`）。
5.  **プロジェクトを公式 ZIP から作り直す**：`src/`・`configuration.xml`・`ra/` を ZIP と `diff -rq` で比較し、差分がプローブとツールチェーン版数だけであることを確認する。
6.  **工場出荷時デモ（Quick Start）を書き込む**：これでも映らなければハード故障と断定できる。

## 18. 工場出荷時デモ（Quick Start Project）の入手と書き込み
*   入手先：`https://github.com/renesas/ra-fsp-examples/releases` の `quickstart_ek_ra8p1_ep.zip`（FSP 6.6.0 なら v6.6.0.example.1）。ビルド済み `.hex` も同梱されている。
*   **他プロジェクトのデバッグセッションから gdb の `load xxx.hex` で書くのは不安定**（OSPI への 5MB 書き込みの途中でタイムアウトし、セッションが落ちた）。
*   確実な手順：ZIP を展開 → e² studio に「既存プロジェクトのインポート」→ Generate Project Content → ビルド（GCC 13.2.1、約3分）→ 付属の `Debug_Flat.launch` でデバッグ実行。
*   **J-Link のインターフェース速度を 4000kHz → 1000kHz に落とす**と通信エラーを避けやすい（launch ファイルの `-uIfSpeed=` と `com.renesas.hardwaredebug.arm.jlink.interface.speed` の両方を書き換える）。
*   書き込み確認（ベリファイ）：
    *   内蔵フラッシュ `x/2wx 0x02000000` → `0x22004100 0x0201ec4d`（2語目はリセットベクタ）。
    *   OSPI `x/2wx 0x90000000`。**リセット直後は OSPI が未初期化で読めない**ので、`main` まで走らせてから読む。
    *   OSPI を 8D-8D-8D（OPI）モードで読むと **16bit 単位でバイト順が入れ替わって見える**（hex の `00 04 58 02` が `0x58020004` に見える）。中身が化けているわけではない。

## 19. EK-RA8P1 のジャンパ／スイッチと LCD の関係
*   **手で変更できる設定のうち、LCD の表示に関係するのは SW4-6 だけ**（OFF = パラレルディスプレイ + MIPI カメラ）。公式サンプルの README は **SW4 を 8 個すべて OFF** にするよう指定している。
*   J16 はブートモード（通常は 2-3）、J6/J8/J9/J29 はデバッガの接続先で、いずれも LCD には無関係。
*   E1〜E62 は基板上の銅パターン（はんだ付け／カット）なので、通常の作業で変わることはない。LCD 信号上の E リンクも無い。
*   グラフィックス拡張ボードは **ピンヘッダ J1** に挿さる。FFC だけでなくこのヘッダの挿し込み（1列ずれていないか）も確認する。
*   SDRAM に関する切り替えスイッチは無い。フレームバッファが SDRAM で正しく読めていれば SDRAM 系は健全。

## 20. 電源・USB ケーブルの切り分け（実際にハマった）
*   **USB Type-C ケーブルの劣化・断線で挙動が変わることがある**。今回は「挿す向き（表裏）で画面が真っ黒／真っ白に変わる」という症状が出て、良品ケーブル（PC 直挿し）に交換したら症状が消えた。
*   ただし今回のケースでは、リセット要因レジスタ（RSTSR1 = 0x4、SWRF のみ／LVD・POR なし）とカメラ・AI が数分間安定動作していたことから、**MCU 自体のブラウンアウトではなかった**。ケーブル交換前の書き込み失敗は、通信の不安定さが原因と考えられる。
*   切り分けの鉄則：ケーブルは必ず良品で、ハブを介さず PC に直挿しする。

## 21. e² studio 操作のハマりどころ（追加）
*   **ブレークポイントはワークスペースに保存され、プロジェクトを作り直しても復活する**。別プロジェクトに置いたブレークポイント（`vin_status_isr` など）が新しいプロジェクトでも効いてしまい、「動いていない」ように見えた。Run →「すべてのブレークポイントを削除」、または Debugger Console で `delete`。
*   **launch ファイルのプロジェクト名がずれると「プログラム・ファイルが存在しません」になる**。ZIP から作り直してプロジェクト名を変えたときは、`.launch` の `org.eclipse.cdt.launch.PROJECT_ATTR` と `<listEntry value="/プロジェクト名"/>` を実際の名前に合わせる。
*   **デバッグ実行は自動でビルドしてくれないことがある**。ソースを変えたら Project →「プロジェクトのビルド」を明示的に実行し、`Debug/*.elf` のタイムスタンプが更新されたことを確認してからデバッグする。
*   e² studio がウィンドウを表示しないまま常駐し、ワークスペースをロックしたままになることがある。タスクマネージャーで `e2studio.exe` を終了させる。
