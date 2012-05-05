#include <stdlib.h>
#include <stdio.h>

#include <list>
#include <string>
#include <sstream>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

typedef std::basic_string<gchar> gstring;
typedef std::list<gstring> gstring_list;
typedef std::basic_stringstream<gchar> gstringstream;

/**
 * メインフレーム
 * @author seraphy
 */
class MainFrame
{
private:

  /**
   * Gtkウィンドウ
   */
  GtkWidget *window;

  /**
   * テキストエリア
   */
  GtkWidget *textarea;

  /**
   * クリップボード形式名入力ボックス
   */
  GtkWidget *formatnamebox;

  /**
   * アクセラレータ
   */
  GtkAccelGroup *accel_group;

public:

  MainFrame()
  {
    // ウィンドウの構築
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    // タイトルとサイズの設定
    gtk_window_set_title(GTK_WINDOW(window), "EnumClipFormat");
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 200);

    // ウィンドウの閉じるイベントのハンドル
    g_signal_connect(G_OBJECT(window),
        "delete_event",
        G_CALLBACK(gtk_main_quit), // 直接、gtk_main_quitを呼ぶ
        NULL);

    // アクセラレータの初期化
    accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

    // コンポーネントの構築
    initComponent();
  }

  /**
   * メインフレームを表示する
   */
  void show()
  {
    gtk_widget_show_all(window);
  }

private:

  /**
   * コンポーネントの初期化
   */
  void initComponent()
  {
    // メニューとコンテンツの基本レイアウト
    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // メニューの構築
    GtkWidget *menubar = gtk_menu_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    GtkWidget *menu = gtk_menu_new();


    GtkWidget *menu_chk_cb = gtk_menu_item_new_with_mnemonic(
        "クリップボードのチェック(_V)");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_chk_cb);

    gtk_widget_add_accelerator(menu_chk_cb,
        "activate", accel_group, GDK_v, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    g_signal_connect(G_OBJECT(menu_chk_cb),
        "activate", G_CALLBACK(chk_cb_event), this);


    GtkWidget *menu_chk_primary = gtk_menu_item_new_with_mnemonic(
        "プライマリセレクションのチェック(_P)");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_chk_primary);

    gtk_widget_add_accelerator(menu_chk_primary,
        "activate", accel_group, GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    g_signal_connect(G_OBJECT(menu_chk_primary),
        "activate", G_CALLBACK(chk_primary_event), this);


    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

    GtkWidget *menu_save = gtk_menu_item_new_with_mnemonic("クリップボードの保存(_S)");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_save);

    gtk_widget_add_accelerator(menu_save,
        "activate", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    g_signal_connect(G_OBJECT(menu_save),
        "activate", G_CALLBACK(save_event), this);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());


    GtkWidget *menu_exit = gtk_menu_item_new_with_mnemonic("終了(_Q)");
    /* menu_exit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group); */
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_exit);

    gtk_widget_add_accelerator(menu_exit,
         "activate", accel_group, GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    g_signal_connect(G_OBJECT(menu_exit),
         "activate", G_CALLBACK(gtk_main_quit), NULL); // 直接、gtk_main_quitを呼ぶ

    GtkWidget *menu_item = gtk_menu_item_new_with_mnemonic("コマンド(_C)");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menu_item);


    // テキストエリア
    textarea = gtk_text_view_new();

    // スクロールペイン
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_container_add(GTK_CONTAINER(scroll), textarea);

    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    // テキストボックス
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    formatnamebox = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), formatnamebox, TRUE, TRUE, 0);

    GtkWidget *save_button = gtk_button_new_with_label("クリップボードを保存");
    gtk_box_pack_start(GTK_BOX(hbox), save_button, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(save_button),
        "clicked", G_CALLBACK(save_event), this);
  }

  /**
   * クリップボードの形式の取得
   */
  void enum_target(GtkClipboard *clipboard)
  {
    GdkAtom *targets = NULL;
    gint n_targets = 0;

    if (gtk_clipboard_wait_for_targets(clipboard, &targets, &n_targets) != TRUE) {
      return;
    }

    // 通知されたクリップボード形式の一覧から
    // 形式名を列挙する
    gstring_list fmtList;

    for (int idx = 0; idx < n_targets; idx++)
    {
      gstring fmtName = gdk_atom_name(targets[idx]);
      fmtList.push_back(fmtName);
    }

    // ソート
    fmtList.sort();

    // テキストバッファに格納する
    gstringstream stm;

    for (gstring_list::const_iterator ite = fmtList.begin();
          ite != fmtList.end(); ++ite)
      stm << *ite << std::endl;

    // テキストビューに書き込む
    GtkTextBuffer *textbuf =
      gtk_text_view_get_buffer(GTK_TEXT_VIEW(textarea));

    gtk_text_buffer_set_text(textbuf, stm.str().c_str(), -1);

    g_free(targets);
  }

  /**
   * クリップボードのチェックメニューハンドラ
   */
  static void chk_cb_event(GtkWidget *widget, gpointer data)
  {
    MainFrame *me = reinterpret_cast<MainFrame*>(data);

    GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    me->enum_target(clipboard);
  }

  /**
   * プライマリセレクションのチェックメニューハンドラ
   */
  static void chk_primary_event(GtkWidget *widget, gpointer data)
  {
    MainFrame *me = reinterpret_cast<MainFrame*>(data);

    GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
    me->enum_target(clipboard);
  }

  /**
   * 保存メニューハンドラ
   */
  static void save_event(GtkWidget *widget, gpointer data)
  {
    MainFrame *me = reinterpret_cast<MainFrame*>(data);

    const gchar *text = gtk_entry_get_text(GTK_ENTRY(me->formatnamebox));
    if (text == NULL || g_utf8_strlen(text, -1) == 0) {
      me->show_message("フォーマット名を入力してください。");
      return;
    }

    // クリップボードから該当データの取り出し
    GdkAtom atom = gdk_atom_intern(text, FALSE);

    GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    GtkSelectionData *seldata = gtk_clipboard_wait_for_contents(clipboard, atom);
    if (seldata == NULL) {
      me->show_message("データはありません。");
      return;
    }

    // 保存先ファイル名の選択
    GtkWidget *savedlg = gtk_file_chooser_dialog_new("Save File",
         GTK_WINDOW(me->window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
         GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (savedlg), TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(savedlg), "clipboard.dat");

    if (gtk_dialog_run(GTK_DIALOG(savedlg)) == GTK_RESPONSE_ACCEPT) {
      gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (savedlg));

      // 該当形式のデータをファイルに保存
      gint siz = gtk_selection_data_get_length(seldata);
      const guchar *binary = gtk_selection_data_get_data(seldata);

      FILE *fp = fopen(filename, "wb");
      fwrite(binary, 1, siz, fp);
      fclose(fp);

      g_free (filename);
    }
    gtk_widget_destroy(savedlg);

    gtk_selection_data_free(seldata);
  }

  /**
   * メッセージボックスの表示
   */
  void show_message(const gchar *message)
  {
    GtkWidget *msgdlg = gtk_message_dialog_new(
        GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
        "%s", message);
    gtk_dialog_run(GTK_DIALOG(msgdlg));
    gtk_widget_destroy(msgdlg);
  }
};

/**
 * エントリポイント
 */
int main(int argc, char *argv[])
{
  gtk_set_locale();
  gtk_init(&argc, &argv);

  MainFrame mainFrame;
  mainFrame.show();

  gtk_main();

  return 0;
}
