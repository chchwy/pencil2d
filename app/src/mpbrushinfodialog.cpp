#include "mpbrushinfodialog.h"

#include <QLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QToolBar>
#include <QMessageBox>

#include <QClipboard>
#include <QMimeData>
#include <QApplication>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "editor.h"
#include <pencilerror.h>
#include "filedialogex.h"
#include "combobox.h"
#include "mpbrushutils.h"

MPBrushInfoDialog::MPBrushInfoDialog(DialogContext dialogContext, QWidget* parent)
    : QDialog(parent), mDialogContext(dialogContext)
{

    setWindowTitle(tr("Edit brush information"));

    QVBoxLayout* vMainLayout = new QVBoxLayout();
    QVBoxLayout* vMain2Layout = new QVBoxLayout();
    QHBoxLayout* hMainLayout = new QHBoxLayout();

    mImageLabel = new QLabel();
    mNameTextEdit = new QPlainTextEdit();
    mNameTextEdit->setMinimumSize(QSize(100,30));
    mNameTextEdit->setMaximumSize(QSize(200,30));
    mNameTextEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);

    mPresetTextEdit = new QPlainTextEdit();
    mPresetTextEdit->setMinimumSize(QSize(100,30));
    mPresetTextEdit->setMaximumSize(QSize(200,30));
    mPresetTextEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);

    mCommentTextEdit = new QPlainTextEdit();
    mCommentTextEdit->setMinimumSize(QSize(200,100));
    mCommentTextEdit->setMaximumSize(QSize(100,100));
    mCommentTextEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    mVersionTextEdit = new QPlainTextEdit();
    mVersionTextEdit->setMinimumSize(QSize(100,30));
    mVersionTextEdit->setMaximumSize(QSize(200,30));
    mVersionTextEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);

    mShowInToolComboBox = new ComboBox();

    mSetImageButton = new QPushButton();
    mSetImageFromClipBoard = new QPushButton();
    mSetImageFromClipBoard->setText(tr("image from clipboard"));

    QToolBar* toolbar = new QToolBar();
    vMainLayout->setContentsMargins(5,5,5,0);

    QPushButton* saveButton = new QPushButton();
    saveButton->setText(tr("Save"));

    QPushButton* cancelButton = new QPushButton();
    cancelButton->setText(tr("Cancel"));

    QWidget* toolbarSpacer = new QWidget(this);
    toolbarSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    vMain2Layout->addSpacerItem(toolbarSpacer);

    toolbar->addWidget(toolbarSpacer);
    toolbar->addWidget(saveButton);
    toolbar->addWidget(cancelButton);

    QLabel* nameLabel = new QLabel();
    nameLabel->setText(tr("Name"));

    QLabel* presetLabel = new QLabel();
    presetLabel->setText(tr("Preset"));
    presetLabel->setToolTip(tr("In which preset do you want this brush displayed"));
    QLabel* descriptionLabel = new QLabel();
    descriptionLabel->setText(tr("Description"));

    QLabel* comboDescriptionLabel = new QLabel();
    comboDescriptionLabel->setText(tr("Show brush in tool"));

    QLabel* versionLabel = new QLabel();
    versionLabel->setText(tr("Version"));

    QVBoxLayout* vRightLayout = new QVBoxLayout();

    vMainLayout->addLayout(hMainLayout);
    hMainLayout->addLayout(vMain2Layout);
    vMain2Layout->addWidget(mImageLabel);
    vMain2Layout->addWidget(mSetImageButton);
    vMain2Layout->addWidget(mSetImageFromClipBoard);
    vMainLayout->addWidget(toolbar);
    mSetImageButton->setText(tr("Add image"));

    QSpacerItem* vSpacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    vMain2Layout->addSpacerItem(vSpacer);

    hMainLayout->addLayout(vRightLayout);

    vRightLayout->addWidget(nameLabel);
    vRightLayout->addWidget(mNameTextEdit);

    vRightLayout->addWidget(presetLabel);
    vRightLayout->addWidget(mPresetTextEdit);

    vRightLayout->addWidget(descriptionLabel);
    vRightLayout->addWidget(mCommentTextEdit);
    vRightLayout->addWidget(versionLabel);
    vRightLayout->addWidget(mVersionTextEdit);
    vRightLayout->addWidget(comboDescriptionLabel);
    vRightLayout->addWidget(mShowInToolComboBox);

    mImageLabel->setText(("Image here"));
    mImageLabel->setAlignment(Qt::AlignCenter);

    mImageLabel->setStyleSheet("QLabel {"
                               "border: 1px solid;"
                               "}");
    mImageLabel->setMinimumSize(QSize(128,128));
    mImageLabel->setMaximumSize(QSize(128,128));

    connect(mSetImageButton, &QPushButton::pressed, this, &MPBrushInfoDialog::didPressSetImage);
    connect(mSetImageFromClipBoard, &QPushButton::pressed, this, &MPBrushInfoDialog::didPressSetImageFromClipBoard);
    connect(mShowInToolComboBox, &ComboBox::activated, this, &MPBrushInfoDialog::didSelectToolOption);
    connect(cancelButton, &QPushButton::pressed, this, &MPBrushInfoDialog::didPressCancel);
    connect(saveButton, &QPushButton::pressed, this, &MPBrushInfoDialog::didPressSave);

    connect(mNameTextEdit, &QPlainTextEdit::textChanged, this, &MPBrushInfoDialog::didUpdateName);
    connect(mCommentTextEdit, &QPlainTextEdit::textChanged, this, &MPBrushInfoDialog::didUpdateComment);
    connect(mVersionTextEdit, &QPlainTextEdit::textChanged, this, &MPBrushInfoDialog::didUpdateVersion);
    connect(mPresetTextEdit, &QPlainTextEdit::textChanged, this, &MPBrushInfoDialog::didUpdatePreset);

    setLayout(vMainLayout);

    if (dialogContext == Clone) {
        saveButton->setText(tr("Clone"));
    }
}

void MPBrushInfoDialog::initUI()
{
    QList<ToolType> toolTypes;
    toolTypes.append(ToolType::PEN);
    toolTypes.append(ToolType::PENCIL);
    toolTypes.append(ToolType::BRUSH);
    toolTypes.append(ToolType::ERASER);
    toolTypes.append(ToolType::SMUDGE);
    toolTypes.append(ToolType::POLYLINE);

    for (ToolType toolType: toolTypes) {
        mShowInToolComboBox->addItem(mEditor->getTool(toolType)->typeName(), static_cast<int>(toolType));
    }
}

void MPBrushInfoDialog::setBrushInfo(QString brushName, QString brushGroup, ToolType tool, QJsonDocument brushJsonDoc)
{
    auto status = MPBrushParser::readBrushFromFile(brushGroup, brushName);

    if (status.errorcode != Status::OK) {

        QMessageBox::warning(this, tr("Parse error"), tr("Could not read brush file data"));
        // TODO: better error handling
        return;
    }

    mBrushName = brushName;

    mBrushPreset = brushGroup;
    mOriginalPreset = brushGroup;

    mOriginalName = brushName;

    mBrushInfoObject = brushJsonDoc.object();

    mBrushInfo = mBrushInfo.read(mBrushInfoObject);

    QString imagePath = MPBrushParser::getBrushPreviewImagePath(mOriginalPreset, mBrushName);
    QPixmap imagePix(imagePath);
    mImageLabel->setPixmap(imagePix);

    mNameTextEdit->setPlainText(brushName);
    mPresetTextEdit->setPlainText(brushGroup);
    mCommentTextEdit->setPlainText(mBrushInfo.comment);
    mVersionTextEdit->setPlainText(QString::number(mBrushInfo.version));

    mToolName = mShowInToolComboBox->currentText();

    mShowInToolComboBox->setCurrentItemFrom(static_cast<ToolType>(tool));
}

void MPBrushInfoDialog::didPressSetImage()
{
    FileDialog fileDialog(this);
    QString strFilePath = fileDialog.openFile(FileType::IMAGE);

    if (strFilePath.isEmpty()) { return; }

    QPixmap imagePix(strFilePath);
    imagePix = imagePix.scaled(mImageLabel->size());
    mImageLabel->setPixmap(imagePix);

    mIconModified = true;
}

void MPBrushInfoDialog::didPressSetImageFromClipBoard()
{
    const QClipboard* clipBoard = QApplication::clipboard();
    const QMimeData *mimeData = clipBoard->mimeData();

    if (mimeData->hasImage()) {
        QPixmap pix = qvariant_cast<QPixmap>(mimeData->imageData());
        QPixmap imagePix(pix);
        imagePix = imagePix.scaled(mImageLabel->size(), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
        mImageLabel->setPixmap(imagePix);
    }

    mIconModified = true;
}

void MPBrushInfoDialog::didSelectToolOption(int index, QString itemName, int value)
{
    Q_UNUSED(index)
    ToolType toolType = static_cast<ToolType>(value);
    mToolName = itemName;
    mToolType = toolType;
}

void MPBrushInfoDialog::didPressCancel()
{
    close();
}

void MPBrushInfoDialog::didPressSave()
{

    QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Save changes"),
                                   tr("Are you sure you want to save changes?"),
                                   QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);

    if (ret == QMessageBox::Yes) {
        mBrushInfo.write(mBrushInfoObject);

        QJsonDocument doc = QJsonDocument(mBrushInfoObject);

        // Change spaces with underscores
        QString noSpaceName = mBrushName.replace(QRegExp("[ ]"), "_");
        QString noSpacePreset = mBrushPreset.replace(QRegExp("[ ]"), "_");

        Status status = Status::OK;
        if (mDialogContext == DialogContext::Clone) {

            status = MPBrushParser::copyRenameBrushFileIfNeeded(mOriginalPreset, mOriginalName, noSpacePreset, noSpaceName);

            if (mIconModified && status.ok()) {
                status = MPBrushParser::writeBrushIcon(*mImageLabel->pixmap(), noSpacePreset, noSpaceName);
            }

            if (status.code() != Status::OK) {
                QMessageBox::warning(this, status.title(),
                                                   status.description());
                return;
            }
        } else { // Edit
            MPBrushParser::renameMoveBrushFileIfNeeded(mOriginalPreset, mOriginalName, noSpacePreset, noSpaceName);
        }
        status = MPBrushParser::writeBrushToFile(noSpacePreset, noSpaceName, doc.toJson());

        if (status.ok()) {
            MPBrushParser::addBrushFileToList(mToolName, noSpacePreset, noSpaceName);
        }

        emit updatedBrushInfo(noSpaceName, noSpacePreset);
        close();
    }
}

void MPBrushInfoDialog::didUpdateName()
{
    mBrushName = mNameTextEdit->toPlainText();
}

void MPBrushInfoDialog::didUpdateComment()
{
    mBrushInfo.comment = mCommentTextEdit->toPlainText();
}

void MPBrushInfoDialog::didUpdateVersion()
{
    mBrushInfo.version = mVersionTextEdit->toPlainText().toDouble();
}

void MPBrushInfoDialog::didUpdatePreset()
{
    mBrushPreset = mPresetTextEdit->toPlainText();
}
