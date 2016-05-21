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

#include "nodes-model.h"

#include <QFont>

NodesModel::NodesModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

NodesModel::NodesModel(ControlWidgetInfoList nodes, QObject* parent)
    : QAbstractTableModel(parent)
{
    m_nodes = nodes;
}

int NodesModel::rowCount(const QModelIndex& /*parent*/) const
{
    return m_nodes.count();
}

int NodesModel::columnCount(const QModelIndex& /*parent*/) const
{
    return COLS;
}

QVariant NodesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_nodes.count() || index.row() < 0)
        return QVariant();

    int row = index.row();
    int col = index.column();

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (col)
        {
        case COL_NAME:
            return m_nodes[row]->name;
        case COL_CHAN:
            return QString(m_nodes[row]->channel);
        case COL_UNIT:
            return m_nodes[row]->unit;
        case COL_HORZ:
            return m_nodes[row]->horizontal ? "H" : "V";
//              case COL_ONDE: return QString(role==Qt::DisplayRole?"%1 %":"%1").arg(m_nodes[row]->preferredLighting);
        case COL_ONDE:
            return m_nodes[row]->preferredLighting;
        case COL_OFTI:
            return m_nodes[row]->switchOffPeriodMn;
        case COL_ROW:
            return m_nodes[row]->row;
        case COL_COL:
            return m_nodes[row]->col;
        }
        break;
    case Qt::FontRole:
        if (col == COL_NAME) //change font only for col 0
        {
            QFont boldFont;
            boldFont.setBold(true);
            return boldFont;
        }
        break;
    case Qt::TextAlignmentRole:
        if (col >= COL_ONDE && col <= COL_OFTI) //change text alignment only for cols 4,5
        {
            return Qt::AlignRight + Qt::AlignVCenter;
        }
        break;
    case Qt::CheckStateRole:
        switch (col)
        {
        case COL_HORZ:
            return m_nodes[row]->horizontal ? Qt::Checked : Qt::Unchecked;
        case COL_BTOF:
            return (m_nodes[row]->style & ControlWidget::ONOFF) ? Qt::Checked : Qt::Unchecked;
        case COL_BSLI:
            return (m_nodes[row]->style & ControlWidget::SLIDER) ? Qt::Checked : Qt::Unchecked;
        case COL_BTUD:
            return (m_nodes[row]->style & ControlWidget::DIMBRIGHT) ? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    return QVariant();
}

QVariant NodesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case COL_NAME:
                return QString("Name");
            case COL_CHAN:
                return QString("House");
            case COL_UNIT:
                return QString("Unit");
            case COL_HORZ:
                return QString("Hz");
            case COL_ONDE:
                return QString("on %");
            case COL_OFTI:
                return QString("x mn");
            case COL_BTOF:
                return QString("OF");
            case COL_BSLI:
                return QString("SL");
            case COL_BTUD:
                return QString("UP");
            case COL_ROW :
                return QString("Row");
            case COL_COL :
                return QString("Col");
            }
        }
    }
    else if (role == Qt::ToolTipRole)
    {
        switch (section)
        {
        case COL_NAME:
            return QString("Name of the room");
        case COL_CHAN:
            return QString("House Code [A-P]");
        case COL_UNIT:
            return QString("Unit Code [1-16]");
        case COL_HORZ:
            return QString("Horizontal Alignment");
        case COL_ONDE:
            return QString("Default Lighting level when turning on");
        case COL_OFTI:
            return QString("Switch Off after period");
        case COL_BTOF:
            return QString("On / Of Buttons");
        case COL_BSLI:
            return QString("Slider control");
        case COL_BTUD:
            return QString("Up / Down Buttons");
        case COL_ROW :
            return QString("Row Position");
        case COL_COL :
            return QString("Column Position");
        }
    }
    return QVariant();
}

Qt::ItemFlags NodesModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flag = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    switch (index.column())
    {
    case COL_HORZ:
    case COL_BTOF:
    case COL_BSLI:
    case COL_BTUD:
        return flag | Qt::ItemIsUserCheckable;
    default:
        return flag | Qt::ItemIsEditable;
    }
}

bool NodesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    int row = index.row();

    if (role == Qt::EditRole)
    {
        switch (index.column())
        {
        case COL_NAME:
            // name is all yours
            m_nodes[row]->name = value.toString();
            break;
        case COL_CHAN:
        {
            // force value to 1 upper char [A-P]
            QChar val = value.toString()[0].toUpper();
            if (val > 'P') val = 'P';
            if (val < 'A') val = 'A';
            m_nodes[row]->channel = val;
            break;
        }
        case COL_UNIT:
        {
            // force unit to [0-16]
            int val = value.toInt();
            if (val > 16) val = 16;
            if (val < 0) val = 0;
            m_nodes[row]->unit = val;
            break;
        }
        case COL_ONDE:
        {
            // On Default Ligting ... in % so force value in range
            int val = value.toString().remove("%").toInt();
            if (val > 100) val = 100;
            if (val < 0) val = 0;
            m_nodes[row]->preferredLighting = val;
            break;
        }
        case COL_OFTI:
        {
            // default time out... prepend mn, fix negative
            int val = value.toString().remove("mn").toInt();
            if (val < 0) val = 0;
            m_nodes[row]->switchOffPeriodMn = val;
            break;
        }
        case COL_ROW:   // row pos
            m_nodes[row]->row = value.toInt();
            break;
        case COL_COL:   // row col
            m_nodes[row]->col = value.toInt();
            break;
        default:
            return false;
        }

        emit editCompleted("EditRole");
        return true;
    }
    else if (role == Qt::CheckStateRole)
    {
        switch (index.column())
        {
        case COL_HORZ:  // slider direction
            m_nodes[row]->horizontal = value.toBool();
            break;
        case COL_BTOF:  // again for On Off
            if (value.toBool())
                m_nodes[row]->style |= ControlWidget::ONOFF;
            else
                m_nodes[row]->style &= ~ControlWidget::ONOFF;
            break;
        case COL_BSLI:  // slider yes or no
            if (value.toBool())
                m_nodes[row]->style |= ControlWidget::SLIDER;
            else
                m_nodes[row]->style &= ~ControlWidget::SLIDER;
            break;
        case COL_BTUD:  // same inquisition about Button Up Down
            if (value.toBool())
                m_nodes[row]->style |= ControlWidget::DIMBRIGHT;
            else
                m_nodes[row]->style &= ~ControlWidget::DIMBRIGHT;
            break;
        default:
            return false;
        }

        emit editCompleted("CheckStateRole");
        return true;
    }

    return false;
}

int NodesModel::findRowAvailable()
{
    return m_nodes.count() + 1;
}

bool NodesModel::findUnitAvailable(const QChar& channel, int* pUnit)
{
    bool found;

    for (int unit = 1; unit < 17; unit++)
    {
        found = false;
        for (int i = 0; i < m_nodes.count(); i++)
        {
            if (m_nodes[i]->channel != channel)
                continue;

            if (unit == m_nodes[i]->unit)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            // update unit with available slot
            *pUnit = unit;
            return true;
        }
    }
    //      if (found) {} // will still be @ 1

    return false;
}

void NodesModel::findUnitAvailable(QChar* pChannel, int* pUnit)
{
    if (m_nodes.count() > 0)
    {
        // take arbitrary channel TODO -> selection
        *pChannel = m_nodes[0]->channel;
    }

    for (int i = 0; i < 16; i++)
    {
        if (findUnitAvailable(*pChannel, pUnit))
            return;
        *pChannel = (*pChannel).toAscii() + 1;
        if ((*pChannel) == 'Q')
            *pChannel = 'A'; // cycle
    }
}


bool NodesModel::insertRows(int position, int rows, const QModelIndex& index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; row++)
    {
        ControlWidgetInfo* newWidget = new ControlWidgetInfo();

        // place on new row
        newWidget->row = findRowAvailable();
        findUnitAvailable(&newWidget->channel, &newWidget->unit);

//      newWidget->CreateWidget();
        m_nodes.insert(position, newWidget);
    }

    endInsertRows();

    emit editCompleted("insertRows");

    return true;
}


bool NodesModel::removeRows(int position, int rows, const QModelIndex& index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
    {
        delete m_nodes.at(position);
        m_nodes.removeAt(position);
    }
    endRemoveRows();

    emit editCompleted("removeRows");

    return true;
}

void NodesModel::writeConfig(KConfigGroup cg)
{
    int i = 0;
    QList<ControlWidgetInfo*>::const_iterator ctrlListIt;
    for (ctrlListIt = m_nodes.begin(); ctrlListIt != m_nodes.end(); ++ctrlListIt, i++)
        (*ctrlListIt)->writeConfig(i, cg);
}

void NodesModel::readConfig(KConfigGroup cg)
{
    QStringList nodes = cg.keyList();

    foreach (const QString & node, nodes)
    {
        ControlWidgetInfo* ctrlIt = new ControlWidgetInfo();
        ctrlIt->readConfig(node.toInt(), cg);

        if (ctrlIt->name != "") // valid
            m_nodes.append(ctrlIt);
    }
}
