#ifndef CPROJECT_ITEM_H
#define CPROJECT_ITEM_H

#include <QtXml>
#include <QTextStream>
#include <QString>

class CProjectItem
{
public:

    CProjectItem();

    CProjectItem(const QString& name, const QString& srcDir, const QString& srcMask, const QString& headerMask,
						   const QString& tagUpdateDateTime, const QString& projectCreateDateTime, const QString& labels);

    QString name_,
            srcDir_,
            srcMask_,
            headerMask_,

			tagUpdateDateTime_,
			projectCreateDateTime_,

			labels_;

    virtual ~CProjectItem() {};
};

#endif // CPROJECT_ITEM_H




