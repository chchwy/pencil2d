#include "mappingconfiguratorwidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QDebug>

#include "mappingdistributionwidget.h"

MappingConfiguratorWidget::MappingConfiguratorWidget(QString description, qreal min, qreal max, QVector<QPointF> points, int maxPoints, QWidget* parent)
    : QWidget(parent)
{
    mMappingDistributionWidget = new MappingDistributionWidget(min, max, points, this);
    mDescription = description;

    mMappingDistributionWidget->setMaxMappingPoints(maxPoints);

    setupUI();
}

MappingConfiguratorWidget::~MappingConfiguratorWidget()
{
}

MappingConfiguratorWidget::MappingConfiguratorWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void MappingConfiguratorWidget::setupUI()
{
    QWidget* outputDescriptionWidget = new QWidget(this);
    QVBoxLayout* outputDescriptionLayout = new QVBoxLayout(this);

    QLabel* maxOutputDesc = new QLabel("100%", this);
    maxOutputDesc->setAlignment(Qt::AlignTop | Qt::AlignRight);
    maxOutputDesc->setMargin(0);
    QLabel* outputDesc = new QLabel("Output", this);
    outputDesc->setMargin(0);
    QLabel* minOutputDesc = new QLabel("0%", this);
    minOutputDesc->setMargin(0);
    minOutputDesc->setAlignment(Qt::AlignBottom | Qt::AlignRight);

    QToolButton* resetButton = new QToolButton(this);
    resetButton->setIcon(QIcon(":/app/icons/new/trash-changes.png"));

    outputDescriptionWidget->setLayout(outputDescriptionLayout);

    outputDescriptionLayout->addWidget(maxOutputDesc);
    outputDescriptionLayout->addWidget(outputDesc);
    outputDescriptionLayout->addWidget(minOutputDesc);

    QHBoxLayout* inputDescriptionLayout = new QHBoxLayout(this);

    QWidget* inputDescriptionWidget = new QWidget(this);

    QLabel* minInputDescLabel = new QLabel("0%", this);
    minInputDescLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    minInputDescLabel->setMargin(0);
    QLabel* inputDescLabel = new QLabel(mDescription, this);
    inputDescLabel->setAlignment(Qt::AlignHCenter);
    inputDescLabel->setMargin(0);
    QLabel* maxInputDescLabel = new QLabel("100%", this);
    maxInputDescLabel->setMargin(0);
    maxInputDescLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);

    inputDescriptionWidget->setLayout(inputDescriptionLayout);

    inputDescriptionLayout->addWidget(minInputDescLabel);
    inputDescriptionLayout->addWidget(inputDescLabel);
    inputDescriptionLayout->addWidget(maxInputDescLabel);

    QGridLayout* layoutGrid = new QGridLayout(this);
    layoutGrid->setMargin(0);
    layoutGrid->setSpacing(0);

    layoutGrid->addWidget(resetButton, 1, 0, 1, 1, Qt::AlignCenter);
    layoutGrid->addWidget(outputDescriptionWidget, 0,0,1,1);

    if (mMappingDistributionWidget) {
        layoutGrid->addWidget(mMappingDistributionWidget, 0,1, 1, 1);

        connect(mMappingDistributionWidget, &MappingDistributionWidget::mappingUpdated, this, &MappingConfiguratorWidget::updateMapping);
        connect(resetButton, &QToolButton::pressed, mMappingDistributionWidget, &MappingDistributionWidget::resetMapping);

    } else {
        QWidget* cosmetic = new QWidget(this);
        cosmetic->setStyleSheet("border: 1px solid black");
        cosmetic->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        minInputDescLabel->setStyleSheet("QLabel { color : gray; }");
        maxInputDescLabel->setStyleSheet("QLabel { color : gray; }");
        minOutputDesc->setStyleSheet("QLabel { color : gray; }");
        maxOutputDesc->setStyleSheet("QLabel { color : gray; }");
        outputDesc->setStyleSheet("QLabel { color : gray; }");
        layoutGrid->addWidget(cosmetic, 0,1, 1, 1);
        resetButton->setDisabled(true);
    }
    layoutGrid->addWidget(inputDescriptionWidget, 1,1, 1, 1);

    setLayout(layoutGrid);
}

void MappingConfiguratorWidget::updateMapping(QVector<QPointF> points)
{
    qDebug() << "mappingConfiguratorWidget - updateMapping called";
    emit mappingUpdated(points);
}
