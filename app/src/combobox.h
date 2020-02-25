#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>

class ComboBox : public QComboBox
{
    Q_OBJECT
public:
    ComboBox(QWidget* parent = nullptr);

    void addItem(const QString& text);
    void addItem(const QString& text, int value);
    void insertItem(const QString& text, int index, int value);
    void setItemEnabled(int index, bool disable);
    void setCurrentItemFrom(int value);

signals:
    void activated(int index, QString name, int data);

private slots:
    void triggerActivatedVariant(int index);

};

#endif // COMBOBOX_H
