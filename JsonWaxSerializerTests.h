#ifndef JSONWAXSERIALIZERTESTS_H
#define JSONWAXSERIALIZERTESTS_H

/* Original author: Nikolai S | https://github.com/doublejim
 *
 * You may use this file under the terms of any of these licenses:
 * GNU General Public License version 2.0       https://www.gnu.org/licenses/gpl-2.0.html
 * GNU General Public License version 3         https://www.gnu.org/licenses/gpl-3.0.html
 */

#include <QObject>
#include <QDate>
#include <QColor>
#include <QImage>
#include <QFont>
#include <QSize>

class SerializerClass1 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(int superNumber MEMBER m_number)
    Q_PROPERTY(Enume1 walker MEMBER m_enume1)
    Q_PROPERTY(Enume2 fish MEMBER m_enume2)
    Q_PROPERTY(QDate bestDate MEMBER m_date)
    Q_PROPERTY(QColor coolColor MEMBER m_color)
    Q_PROPERTY(QImage imageAttempt MEMBER m_image)          // Can't save its data.
    Q_ENUMS(Enume1 Enume2)                                  // Required to store the enums.

public:
    SerializerClass1( QObject* parent = 0): QObject(parent){}

    enum Enume1 {Crab, Spider};
    enum Enume2 {Salmon, Trout, Mackerel};

private:
    QString m_name = "One\nWonderful Name";
    int m_number = 1900000;
    Enume1 m_enume1 = Crab;
    Enume2 m_enume2 = Trout;
    QDate m_date = QDate(1950,10,10);
    QColor m_color = QColor(15,16,16,255);
    QImage m_image = QImage(512,512,QImage::Format_Mono);
};

// ------------------------------------------------------

class SerializerClass2 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(int superNumber READ number WRITE setNumber)
    Q_PROPERTY(Enume1 walker READ enume1 WRITE setEnume1)
    Q_PROPERTY(Enume2 fish READ enume2 WRITE setEnume2)
    Q_PROPERTY(QDate bestDate READ date WRITE setDate)
    Q_PROPERTY(QColor coolColor READ color WRITE setColor)
    Q_PROPERTY(QImage imageAttempt READ image WRITE setImage)           // Can't save anything.
    Q_ENUMS(Enume1 Enume2)                                              // Required to store the enums.

public:
    SerializerClass2( QObject* parent = 0): QObject(parent){}

    enum Enume1 {Crab, Spider};
    enum Enume2 {Salmon, Trout, Mackerel};

    void setName( QString const& name)
    {
        m_name = name;
    }

    void setNumber(const int number)
    {
        m_number = number;
    }

    void setEnume1(Enume1 value)
    {
        m_enume1 = value;
    }

    void setEnume2(Enume2 value)
    {
        m_enume2 = value;
    }

    void setDate(const QDate& date)
    {
        m_date = date;
    }

    void setColor(const QColor& color)
    {
        m_color = color;
    }

    void setImage(const QImage& image)
    {
        m_image = image;
    }

    int number() const
    {
        return m_number;
    }

    Enume1 enume1() const
    {
        return m_enume1;
    }

    Enume2 enume2() const
    {
        return m_enume2;
    }

    QDate date() const
    {
        return m_date;
    }

    QString name() const
    {
        return m_name;
    }

    QColor color() const
    {
        return m_color;
    }

    QImage image() const
    {
        return m_image;
    }

private:
    QString m_name = "One\nWonderful Name";
    int m_number = 1900000;
    Enume1 m_enume1 = Crab;
    Enume2 m_enume2 = Trout;
    QDate m_date = QDate(1950,10,10);
    QColor m_color = QColor(15,16,16,255);
    QImage m_image = QImage(512,512,QImage::Format_Mono);
};

#endif // JSONWAXSERIALIZERTESTS_H
