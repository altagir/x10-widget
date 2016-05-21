/*
 *  Copyright (C) 2013 Sébastien Sénéchal <altagir@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NODES_MODEL_H
#define NODES_MODEL_H

#include <QAbstractItemModel>
#include <QItemDelegate>

#include "control-widgetinfo.h"

const int COLS = 11;

enum COLUMNS
{
    COL_NAME = 0,
    COL_CHAN = 1,
    COL_UNIT = 2,
    COL_HORZ = 3,
    COL_ONDE = 4,
    COL_OFTI = 5,
    COL_BTOF = 6,
    COL_BSLI = 7,
    COL_BTUD = 8,
    COL_ROW  = 9,
    COL_COL  = 10
};

class NodesModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    NodesModel(QObject* parent = 0);
    NodesModel(ControlWidgetInfoList nodes, QObject* parent);

    ControlWidgetInfoList* Nodes() {
        return &m_nodes;
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const   ;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex& index) const ;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

    virtual bool insertRows(int position, int rows, const QModelIndex& index = QModelIndex());
    virtual bool removeRows(int position, int rows, const QModelIndex& index = QModelIndex());

    // works all by itself except checkbox
//      virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    void writeConfig(KConfigGroup cg);
    void readConfig(KConfigGroup cg);

private:
    ControlWidgetInfoList m_nodes;

    bool findUnitAvailable(const QChar& channel, int* pUnit);
    void findUnitAvailable(QChar* channel, int* unit);
    int  findRowAvailable();

signals:
    void editCompleted(const QString&);
};

#endif // NODES_MODEL_H


