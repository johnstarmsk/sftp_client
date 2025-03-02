#ifndef SFTP_CLIENT_H
#define SFTP_CLIENT_H

#include <QMainWindow>

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QTextEdit>
#include <QFileDialog>
#include <QThread>
#include <QProgressBar>
#include <QGroupBox>
#include <string>
#include <QFileDialog>
#include <QTimer>

#include <curl/curl.h>
#include <stdio.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

//--------------------------------------------------------------------

struct FtpFile {
    const wchar_t *filename;
    FILE *stream;
};

//--------------------------------------------------------------------

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct FtpFile *out = (struct FtpFile *)stream;
    if(!out->stream) {
        out->stream = _wfopen(out->filename, L"wb");
        if (!out->stream)
            return 0;
    }
    return fwrite(buffer, size, nmemb, out->stream);
}

static size_t my_read(char *ptr, size_t size, size_t nmemb, void *stream)
{
    FILE *f = (FILE *)stream;
    size_t n;

    if(ferror(f))
        return CURL_READFUNC_ABORT;

    n = fread(ptr, size, nmemb, f) * size;

    return n;
}

struct ProgressData {
    double total_size;
    double downloaded;
    double total_size_upload;
    double uploaded;
};

static int progress_callback(void *ptr, double total_to_download, double now_downloaded, double total_to_upload, double now_uploaded) {
    struct ProgressData *progress = (struct ProgressData *)ptr;

    progress->total_size = total_to_download;
    progress->downloaded = now_downloaded;
    progress->total_size_upload = total_to_upload;
    progress->uploaded = now_uploaded;

    return 0;
}


//--------------------------------------------------------------------

class DownloadThread : public QThread
{
    Q_OBJECT

protected:
    void run() override;
public:
    std::wstring filename;
    QString url;
    QString user_pwd;
    QString user_name;
    QString ssh_key;
    QString key_passphrase;
    QTextEdit* console;
    ProgressData progress_data;
};

//--------------------------------------------------------------------

class UploadThread : public QThread
{
    Q_OBJECT

protected:
    void run() override;
public:
    std::wstring filename;
    QString url;
    QString user_pwd;
    QString user_name;
    QString ssh_key;
    QString key_passphrase;
    QTextEdit* console;
    ProgressData progress_data;
};

//--------------------------------------------------------------------

class MainWidget : public QWidget
{
    Q_OBJECT

public slots:
    void download_slot();
    void upload_slot();
    void load_key_file_slot();
    void save_path_slot();
    void download_finished_slot();
    void upload_finished_slot();
    void update_download_upload_progressbar_slot();

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

    QString ip_read_from_file;
    QString user_read_from_file;
    QString download_filename_read_from_file;
    QString upload_filename_read_from_file;

    QGridLayout* grid;

    QLabel* ip_lbl;
    QLineEdit* ip_input;

    QLabel* user_lbl;
    QLabel* pass_lbl;
    QLineEdit* user_input;
    QLineEdit* pass_input;

    QLabel* path_lbl;
    QLineEdit* path;

    QPushButton* load_key_file_btn;
    QLabel* load_key_file_btn_lbl;

    QLabel* passphrase_lbl;
    QLineEdit* passphrase_input;

    QPushButton* save_path;
    QLabel* save_path_lbl;

    QPushButton* download;
    QPushButton* upload_btn;

    QLabel* upload_filename_lbl;
    QLineEdit* upload_filename_input;

    QTextEdit* console;

    QProgressBar* progress;

    QTimer* update_download_progressbar;

private:
    QString save_path_name;
    DownloadThread* download_thread;
    UploadThread* upload_thread;

};

//--------------------------------------------------------------------

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    MainWidget* main_widget;

private:
    Ui::MainWindow *ui;
};
#endif // SFTP_CLIENT_H
