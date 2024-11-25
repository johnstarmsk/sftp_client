#include "sftp_client.h"
#include "ui_sftp_client.h"

void DownloadThread::run()
{
    CURL *curl;
    CURLcode res;

    struct FtpFile ftpfile = {
        filename.c_str(),
        NULL
    };

    progress_data = {0, 0, 0, 0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, user_pwd.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, user_name.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_SSH_PRIVATE_KEYFILE, ssh_key.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_KEYPASSWD, key_passphrase.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progress_data);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            console->append(QString(curl_easy_strerror(res)));
        }
        else {
            console->append("Download complete");
        }

        curl_easy_cleanup(curl);
    }

    if (ftpfile.stream)
        fclose(ftpfile.stream);

    curl_global_cleanup();
}

//---------------------------------------------------

void UploadThread::run()
{
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    progress_data = {0, 0, 0, 0};

    if (curl) {

        curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, user_pwd.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, user_name.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_SSH_PRIVATE_KEYFILE, ssh_key.toStdString().c_str());
        curl_easy_setopt(curl, CURLOPT_KEYPASSWD, key_passphrase.toStdString().c_str());

        FILE *file = _wfopen(filename.c_str(), L"rb");
        if (!file) {
            qDebug() << "Failed to open local file for upload";
            return;
        }
        fseek(file, 0, SEEK_END);
        long filesize = ftell(file);
        rewind(file);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, my_read);
        curl_easy_setopt(curl, CURLOPT_READDATA, file);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_easy_setopt(curl, CURLOPT_INFILESIZE, filesize);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progress_data);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            console->append(QString(curl_easy_strerror(res)));
        }
        else {
            console->append("Upload complete");
        }

        fclose(file);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return;
}

//---------------------------------------------------

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
{
    download_thread = new DownloadThread;
    upload_thread = new UploadThread;

    grid = new QGridLayout();
    grid->setAlignment(Qt::AlignTop);

    ip_lbl = new QLabel("ip: ");
    ip_lbl->setAlignment(Qt::AlignRight);
    ip_input = new QLineEdit("");

    user_lbl = new QLabel("user:");
    user_lbl->setAlignment(Qt::AlignRight);
    user_input = new QLineEdit("");

    pass_lbl = new QLabel("password:");
    pass_lbl->setAlignment(Qt::AlignRight);
    pass_input = new QLineEdit("");
    pass_input->setEchoMode(QLineEdit::Password);

    path_lbl = new QLabel("download file:");
    path_lbl->setAlignment(Qt::AlignRight);
    path = new QLineEdit("Enter your home catalogue (with filename)");

    load_key_file_btn = new QPushButton("SSH key");
    load_key_file_btn_lbl = new QLabel("KEY NOT SELECTED");

    passphrase_lbl = new QLabel("passphrase:");
    passphrase_lbl->setAlignment(Qt::AlignRight);
    passphrase_input = new QLineEdit("");
    passphrase_input->setEchoMode(QLineEdit::Password);

    save_path = new QPushButton("Save path");
    save_path_lbl = new QLabel("");

    download = new QPushButton("Download");
    upload_btn = new QPushButton("Upload");

    upload_filename_lbl = new QLabel("upload filename:");
    upload_filename_lbl->setAlignment(Qt::AlignRight);
    upload_filename_input = new QLineEdit("Enter server home catalogue (with filename)");

    console = new QTextEdit();

    progress = new QProgressBar();
    progress->setValue(0);

    grid->addWidget(ip_lbl, 0, 0);
    grid->addWidget(ip_input, 0, 1);
    grid->addWidget(user_lbl, 1, 0);
    grid->addWidget(user_input, 1, 1);
    grid->addWidget(pass_lbl, 2, 0);
    grid->addWidget(pass_input, 2, 1);
    grid->addWidget(path_lbl, 3, 0);
    grid->addWidget(path, 3, 1);
    grid->addWidget(load_key_file_btn, 4, 0);
    grid->addWidget(load_key_file_btn_lbl, 4, 1);
    grid->addWidget(passphrase_lbl, 5, 0);
    grid->addWidget(passphrase_input, 5, 1);
    grid->addWidget(save_path, 6, 0);
    grid->addWidget(save_path_lbl, 6, 1);
    grid->addWidget(download, 7, 0);
    grid->addWidget(upload_btn, 8, 0);
    grid->addWidget(upload_filename_lbl, 9, 0);
    grid->addWidget(upload_filename_input, 9, 1);
    grid->addWidget(console, 10, 0, 1, 2);
    grid->addWidget(progress, 11, 0, 1, 2);

    setLayout(grid);

    connect(download, SIGNAL(clicked()), this, SLOT(download_slot()));
    connect(upload_btn, SIGNAL(clicked()), this, SLOT(upload_slot()));
    connect(load_key_file_btn, SIGNAL(clicked()), this, SLOT(load_key_file_slot()));
    connect(save_path, SIGNAL(clicked()), this, SLOT(save_path_slot()));
    connect(download_thread, SIGNAL(finished()), this, SLOT(download_finished_slot()));
    connect(upload_thread, SIGNAL(finished()), this, SLOT(upload_finished_slot()));

    update_download_progressbar = new QTimer();
    connect(update_download_progressbar, SIGNAL(timeout()), this, SLOT(update_download_upload_progressbar_slot()));
}

void MainWidget::download_finished_slot()
{
    progress->setValue(0);
    update_download_progressbar->stop();
    ip_input->setEnabled(true);
    user_input->setEnabled(true);
    pass_input->setEnabled(true);
    path->setEnabled(true);
    load_key_file_btn->setEnabled(true);
    passphrase_input->setEnabled(true);
    save_path->setEnabled(true);
    download->setEnabled(true);
    upload_btn->setEnabled(true);
    upload_filename_input->setEnabled(true);
}

void MainWidget::upload_finished_slot()
{
    progress->setValue(0);
    update_download_progressbar->stop();
    ip_input->setEnabled(true);
    user_input->setEnabled(true);
    pass_input->setEnabled(true);
    path->setEnabled(true);
    load_key_file_btn->setEnabled(true);
    passphrase_input->setEnabled(true);
    save_path->setEnabled(true);
    download->setEnabled(true);
    upload_btn->setEnabled(true);
    upload_filename_input->setEnabled(true);
}

void MainWidget::load_key_file_slot()
{
    load_key_file_btn_lbl->setText(QFileDialog::getOpenFileName());
}

void MainWidget::save_path_slot()
{
    save_path_name = QFileDialog::getSaveFileName();
    save_path_lbl->setText(save_path_name);
}

void MainWidget::download_slot()
{
    update_download_progressbar->start(10);
    ip_input->setEnabled(false);
    user_input->setEnabled(false);
    pass_input->setEnabled(false);
    path->setEnabled(false);
    load_key_file_btn->setEnabled(false);
    passphrase_input->setEnabled(false);
    save_path->setEnabled(false);
    download->setEnabled(false);
    upload_btn->setEnabled(false);
    upload_filename_input->setEnabled(false);

    QString filename;
    if (save_path_name != ""){
        filename = save_path_name;
    }
    else {
        filename = path->text().split("/")[path->text().split("/").size() - 1];
        if (filename == path->text()){
            filename = path->text().split("\\")[path->text().split("\\").size() - 1];
        }
    }
    download_thread->filename = filename.toStdWString();
    download_thread->url = "sftp://" + ip_input->text() + "/" + path->text().replace(" ", "%20");
    download_thread->user_pwd = user_input->text() + ":" + pass_input->text();
    download_thread->user_name = user_input->text();
    download_thread->ssh_key = load_key_file_btn_lbl->text();
    download_thread->key_passphrase = passphrase_input->text();
    download_thread->console = console;
    download_thread->start();
}

void MainWidget::update_download_upload_progressbar_slot()
{
    progress->setMinimum(0);
    if (download_thread->isRunning()){
        if (download_thread->progress_data.total_size != 0){
            progress->setMaximum(download_thread->progress_data.total_size);
        }
        progress->setValue(download_thread->progress_data.downloaded);
    }
    else if (upload_thread->isRunning()){
        if (upload_thread->progress_data.total_size_upload != 0){
            progress->setMaximum(upload_thread->progress_data.total_size_upload);
        }
        progress->setValue(upload_thread->progress_data.uploaded);
    }
}

void MainWidget::upload_slot()
{
    ip_input->setEnabled(false);
    user_input->setEnabled(false);
    pass_input->setEnabled(false);
    path->setEnabled(false);
    load_key_file_btn->setEnabled(false);
    passphrase_input->setEnabled(false);
    save_path->setEnabled(false);
    download->setEnabled(false);
    upload_btn->setEnabled(false);
    upload_filename_input->setEnabled(false);

    QString upload_file_str = QFileDialog::getOpenFileName();
    if (upload_file_str != ""){
        update_download_progressbar->start(10);
        upload_thread->filename = upload_file_str.toStdWString();
        upload_thread->url = "sftp://" + ip_input->text() + "/" + upload_filename_input->text().replace(" ", "%20");
        upload_thread->user_pwd = user_input->text() + ":" + pass_input->text();
        upload_thread->user_name = user_input->text();
        upload_thread->ssh_key = load_key_file_btn_lbl->text();
        upload_thread->key_passphrase = passphrase_input->text();
        upload_thread->console = console;
        upload_thread->start();
    }
    else {
        ip_input->setEnabled(true);
        user_input->setEnabled(true);
        pass_input->setEnabled(true);
        path->setEnabled(true);
        load_key_file_btn->setEnabled(true);
        passphrase_input->setEnabled(true);
        save_path->setEnabled(true);
        download->setEnabled(true);
        upload_btn->setEnabled(true);
        upload_filename_input->setEnabled(true);
    }
}

MainWidget::~MainWidget()
{
}

//---------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("SFTP_Client");
    resize(500, 400);

    main_widget = new MainWidget(this);
    setCentralWidget(main_widget);
}

MainWindow::~MainWindow()
{
    delete ui;
}
