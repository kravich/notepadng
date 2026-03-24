#ifndef QT5_POLYFILL_H
#define QT5_POLYFILL_H

#include <QString>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace Qt
{

const QString::SplitBehavior SkipEmptyParts = QString::SkipEmptyParts;

}
#endif

#endif // QT5_POLYFILL_H
