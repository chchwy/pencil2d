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

    vMain2Layout->setContentsMargins(0,2,2,0);
    hMainLayout->setContentsMargins(0,0,0,0);

    mImageLabel = new QLabel();
    mNameTextEdit = new QPlainTextEdit();
    mNameTextEdit->setMinimumSize(QSize(100,30));
    mNameTextEdit->setMaximumSize(QSize(200,30));
    mNameTextEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);

    mPresetComboBox = new ComboBox(this);

    mCommentTextEdit = new QPlainTextEdit();
    mCommentTextEdit->setMinimumSize(QSize(200,100));
    mCommentTextEdit->setMaximumSize(QSize(100,100));
    mCommentTextEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    mVersionTextEdit = new QPlainTextEdit();
    mVersionTextEdit->setMinimumSize(QSize(100,30));
    mVersionTextEdit->setMaximumSize(QSize(200,30));
    mVersionTextEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);

    mToolComboBox = new ComboBox();

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

    toolbar->addWidget(toolbarSpacer);
    toolbar->addWidget(saveButton);
    toolbar->addWidget(cancelButton);

    QLabel* nameLabel = new QLabel();
    nameLabel->setText(tr("Name"));

    QLabel* descriptionLabel = new QLabel();
    descriptionLabel->setText(tr("Description"));

    QLabel* brushPlacementDescription = new QLabel(this);
    brushPlacementDescription->setText(tr("Show brush in"));

    QHBoxLayout* presetLayout = new QHBoxLayout();

    presetLayout->setContentsMargins(0,0,0,0);

    QLabel* presetLabel = new QLabel();
    presetLabel->setText(tr("Preset"));
    presetLabel->setToolTip(tr("In which preset do you want this brush displayed"));

    presetLayout->addWidget(presetLabel);
    presetLayout->addWidget(mPresetComboBox);

    mPresetComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Ignored);
    mPresetComboBox->setContentsMargins(0,0,0,0);

    QHBoxLayout* toolLayout = new QHBoxLayout();

    toolLayout->setContentsMargins(0,0,0,0);

    QLabel* toolDescription = new QLabel();
    toolDescription->setText(tr("Tool"));

    toolLayout->addWidget(toolDescription);
    toolLayout->addWidget(mToolComboBox);

    mToolComboBox->setContentsMargins(0,0,0,0);

    mToolComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Ignored);

    QLabel* versionLabel = new QLabel();
    versionLabel->setText(tr("Version"));

    QVBoxLayout* vRightLayout = new QVBoxLayout();

    vMainLayout->addLayout(hMainLayout);
    hMainLayout->addLayout(vMain2Layout);
    vMain2Layout->addWidget(mImageLabel);
    vMain2Layout->addWidget(mSetImageButton);
    vMain2Layout->addWidget(mSetImageFromClipBoard);

    vMain2Layout->addWidget(brushPlacementDescription);
    vMain2Layout->addLayout(presetLayout);
    vMain2Layout->addLayout(toolLayout);
    vMainLayout->addWidget(toolbar);
    mSetImageButton->setText(tr("Add image"));

    QSpacerItem* vSpacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    vMain2Layout->addSpacerItem(vSpacer);

    hMainLayout->addLayout(vRightLayout);

    vRightLayout->addWidget(nameLabel);
    vRightLayout->addWidget(mNameTextEdit);

    vRightLayout->addWidget(descriptionLabel);
    vRightLayout->addWidget(mCommentTextEdit);

    vRightLayout->addWidget(versionLabel);
    vRightLayout->addWidget(mVersionTextEdit);

    mImageLabel->setText(("Image here"));
    mImageLabel->setAlignment(Qt::AlignCenter);

    mImageLabel->setStyleSheet("QLabel {"
                               "border: 1px solid;"
                               "}");
    mImageLabel->setMinimumSize(QSize(64,64));

    connect(mSetImageButton, &QPushButton::pressed, this, &MPBrushInfoDialog::didPressSetImage);
    connect(mSetImageFromClipBoard, &QPushButton::pressed, this, &MPBrushInfoDialog::didPressSetImageFromClipBoard);
    connect(mToolComboBox, &ComboBox::activated, this, &MPBrushInfoDialog::didSelectToolOption);
    connect(cancelButton, &QPushButton::pressed, this, &MPBrushInfoDialog::didPressCancel);
    connect(saveButton, &QPushButton::pressed, this, &MPBrushInfoDialog::didPressSave);

    connect(mNameTextEdit, &QPlainTextEdit::textChanged, this, &MPBrushInfoDialog::didUpdateName);
    connect(mCommentTextEdit, &QPlainTextEdit::textChanged, this, &MPBrushInfoDialog::didUpdateComment);
    connect(mVersionTextEdit, &QPlainTextEdit::textChanged, this, &MPBrushInfoDialog::didUpdateVersion);
    connect(mPresetComboBox, &ComboBox::activated, this, &MPBrushInfoDialog::didUpdatePreset);

    setLayout(vMainLayout);

    // calculate layout and set it fixed size.
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

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
        mToolComboBox->addItem(mEditor->getTool(toolType)->typeName(), static_cast<int>(toolType));
    }

    QFile fileOrder(MPBrushParser::getBrushConfigPath(BrushConfigFile));

    if (fileOrder.open(QIODevice::ReadOnly))
    {
        // TODO: will probably have to create a brush importer
        auto brushPresets = MPBrushParser::parseConfig(fileOrder, MPCONF::getBrushesPath());

        for (MPBrushPreset preset : brushPresets) {
            mPresetComboBox->addItem(preset.name);
        }
    }
}

void MPBrushInfoDialog::setBrushInfo(QString brushName, QString brushPreset, ToolType tool, QJsonDocument brushJsonDoc)
{
    auto status = MPBrushParser::readBrushFromFile(brushPreset, brushName);

    if (status.errorcode != Status::OK) {

        QMessageBox::warning(this, tr("Parse error"), tr("Could not read brush file data"));
        // TODO: better error handling
        return;
    }

    mBrushName = brushName;

    mBrushPreset = brushPreset;
    mOriginalPreset = brushPreset;

    mOriginalName = brushName;

    mBrushInfoObject = brushJsonDoc.object();

    mBrushInfo = mBrushInfo.read(mBrushInfoObject);

    QString imagePath = MPBrushParser::getBrushPreviewImagePath(mOriginalPreset, mBrushName);
    QPixmap imagePix(imagePath);
    mImageLabel->setPixmap(imagePix);

    mNameTextEdit->setPlainText(brushName);
    mPresetComboBox->setCurrentItemFrom(brushPreset);
    mCommentTextEdit->setPlainText(mBrushInfo.comment);
    mVersionTextEdit->setPlainText(QString::number(mBrushInfo.version));

    mToolComboBox->setCurrentItemFrom(static_cast<ToolType>(tool));
    mToolName = mToolComboBox->currentText();
    mOldToolName = mToolName;
    mOldPresetName = brushPreset;
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

        Status status = Status::OK;
        if (mDialogContext == DialogContext::Clone) {

            status = MPBrushParser::copyRenameBrushFileIfNeeded(mOriginalPreset, mOriginalName, mBrushPreset, noSpaceName);

            if (!status.ok()) {
               QMessageBox::warning(this, status.title(),
                                                  status.description());
               return;
            }

            if (mIconModified) {
                status = MPBrushParser::writeBrushIcon(*mImageLabel->pixmap(), mBrushPreset, noSpaceName);
            }

            if (!status.ok()) {
               QMessageBox::warning(this, status.title(),
                                                  status.description());
               return;
            }
        } else { // Edit
            status = MPBrushParser::renameMoveBrushFileIfNeeded(mOriginalPreset, mOriginalName, mBrushPreset, noSpaceName);

            if (status.fail()) {
                QMessageBox::warning(this, status.title(), status.description());
                return;
            }
        }
        status = MPBrushParser::writeBrushToFile(mBrushPreset, noSpaceName, doc.toJson());

        if (status.fail()) {
            QMessageBox::warning(this, status.title(), status.description());
            return;
        } else {
            status = MPCONF::addBrush(mToolName, mBrushPreset, noSpaceName);

            if (status.fail()) {
                QMessageBox::warning(this, status.title(), status.description());
                return;
            }
        }

        if (status.fail()) {
            QMessageBox::warning(this, status.title(), status.description());
            return;
        } else {
            emit updatedBrushInfo(noSpaceName, mBrushPreset);
            close();
        }
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

void MPBrushInfoDialog::didUpdatePreset(int index, QString name, int data)
{
    mBrushPreset = name;
}
