﻿//
//  CAPickerView.cpp
//  CrossApp
//
//  Created by dai xinping on 14-7-22.
//  Copyright (c) 2014年 www.9miao.com. All rights reserved.
//

#include "CAPickerView.h"
#include "CALabel.h"
#include "platform/CADensityDpi.h"
NS_CC_BEGIN

CAPickerView::CAPickerView()
: m_delegate(nullptr)
, m_dataSource(nullptr)
, m_fontSizeNormal(20)
, m_fontSizeSelected(22)
, m_fontColorNormal(CAColor4B(0, 0, 0, 255))
, m_fontColorSelected(CAColor4B(0, 0, 0, 255))
, m_separateColor(CAColor4B(127, 127, 127, 127))
, m_displayRow(7)
{
    setDisplayRange(false);
}

CAPickerView::~CAPickerView()
{

}

CAPickerView* CAPickerView::create()
{
    CAPickerView* view = new CAPickerView();
    if (view && view->init())
    {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    return view;
}

CAPickerView* CAPickerView::createWithFrame(const DRect& rect)
{
    CAPickerView* view = new CAPickerView();
    if (view && view->initWithFrame(rect))
    {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    return view;
}

CAPickerView* CAPickerView::createWithCenter(const DRect& rect)
{
    CAPickerView* view = new CAPickerView();
    if (view && view->initWithCenter(rect))
    {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    return view;
}

CAPickerView* CAPickerView::createWithLayout(const CrossApp::DLayout &layout)
{
    CAPickerView* view = new CAPickerView();
    if (view && view->initWithLayout(layout))
    {
        view->autorelease();
    } else {
        CC_SAFE_DELETE(view);
    }
    return view;
}

bool CAPickerView::init()
{
    if (!CAView::init())
    {
        return false;
    }
    
    return true;
}


void CAPickerView::onEnterTransitionDidFinish()
{
	CAView::onEnterTransitionDidFinish();
    
    CAScheduler::getScheduler()->scheduleOnce([=](float dt)
    {
        this->reloadAllComponents();
    }, "first_reload_data", this, 0);
}

void CAPickerView::onExitTransitionDidStart()
{
	CAView::onExitTransitionDidStart();
}

void CAPickerView::setContentSize(const DSize& size)
{
    CC_RETURN_IF(m_obContentSize.equals(size));
    CAView::setContentSize(size);
    if (m_bRunning)
    {
        std::vector<int> selected = m_selected;
        
        for (size_t i=0; i<selected.size(); i++)
        {
            if (CATableView* tableView = m_tableViews.at(i))
            {
                int maxRow = 0;
                if (m_obNumberOfRowsInComponentCallback)
                {
                    maxRow = m_obNumberOfRowsInComponentCallback((unsigned int)i);
                }
                else if (m_dataSource)
                {
                    maxRow = m_dataSource->numberOfRowsInComponent(this, (unsigned int)i);
                }
                unsigned int rowHeight = 0;
                if (m_obHeightForComponentCallback)
                {
                    rowHeight = m_obHeightForComponentCallback((unsigned int)i);
                }
                else if (m_dataSource)
                {
                    rowHeight = m_dataSource->rowHeightForComponent(this, (unsigned int)i);
                }
                DPoint offset = DPointZero;
                int row = selected.at(i);
                if (maxRow <= m_displayRow[i])
                {
                    m_selected[i] = row ;
                    offset.y = row * rowHeight;
                    tableView->setContentOffset(offset, false);
                }
                else
                {
                    m_selected[i] = row;
                    offset.y = m_selected[i] * rowHeight + rowHeight/2 - tableView->getBounds().size.height/2;
                    tableView->setContentOffset(offset, false);
                }
            }
        }
    }
}

float CAPickerView::calcTotalWidth(unsigned int component)
{
    float total = 0;
    for (int i=0; i<component; i++)
    {
        unsigned int width = 0;
        if (m_obWidthForComponentCallback)
        {
            width = m_obWidthForComponentCallback(i);
        }
        else if (m_dataSource)
        {
            width = m_dataSource->widthForComponent(this, i);
        }
        
        total += width;
    }
    return total;
}

void CAPickerView::reloadAllComponents()
{
    CC_RETURN_IF(!this->isRunning());
    
    m_tableViews.clear();
    m_selected.clear();
    m_componentsIndex.clear();
    m_displayRow.clear();
    
    // clear all tableviews
    this->removeAllSubviews();
    
    // reload data
    unsigned int component = 1;
    if (m_obNumberOfComponentsCallback)
    {
        component = m_obNumberOfComponentsCallback();
    }
    else if (m_dataSource)
    {
        component = m_dataSource->numberOfComponentsInPickerView(this);
    }
    
    float total_width = calcTotalWidth(component);
    m_componentsIndex.resize(component);
    m_componentOffsetX.resize(component);
    m_displayRow.resize(component);
    float start_x = getFrame().size.width/2 - total_width/2;
    for (int i=0; i<component; i++)
    {
        m_selected.push_back(0);
        m_componentsIndex[i] = std::vector<int>();
        m_componentOffsetX[i] = start_x;
    
        unsigned int rowHeight = 0;
        if (m_obHeightForComponentCallback)
        {
            rowHeight = m_obHeightForComponentCallback(i);
        }
        else if (m_dataSource)
        {
            rowHeight = m_dataSource->rowHeightForComponent(this, i);
        }
        
        m_displayRow[i] = m_obContentSize.height / rowHeight;
        unsigned int tableHeight = MAX(rowHeight, rowHeight * m_displayRow[i]) ;
        unsigned int tableWidth = 0;
        if (m_obWidthForComponentCallback)
        {
            tableWidth = m_obWidthForComponentCallback(i);
        }
        else if (m_dataSource)
        {
            tableWidth = m_dataSource->widthForComponent(this, i);
        }
        
        
        
        if (m_displayRow[i] % 2 == 0)
        {
            m_displayRow[i] += 1;
        }
        
        // create tableview
        
        float start_y = m_obContentSize.height/2 - tableHeight/2;
        
        CATableView* tableView = CATableView::createWithFrame(DRect(start_x, start_y, tableWidth, tableHeight));
        tableView->setTableViewDataSource(this);
        tableView->setSeparatorViewHeight(0);
        tableView->setSeparatorColor(CAColor4B::CLEAR);
        tableView->setShowsScrollIndicators(false);
        this->addSubview(tableView);
        m_tableViews.pushBack(tableView);
        
        // create highlight
        DSize selectSize = DSize(tableWidth, rowHeight);
        
        CAView* selectedView = nullptr;
        if (m_obViewForSelectedCallback)
        {
            selectedView = m_obViewForSelectedCallback(i, selectSize);
        }
        else if (m_dataSource)
        {
            selectedView = m_dataSource->viewForSelect(this, i, selectSize);
        }
        
        if (selectedView == nullptr)
        {
            DRect sepRect = DRect(start_x, m_obContentSize.height/2 - rowHeight/2, tableWidth, s_px_to_dip(1));
            this->addSubview(CAView::createWithFrame(sepRect, m_separateColor));
            sepRect.origin.y += rowHeight;
            this->addSubview(CAView::createWithFrame(sepRect, m_separateColor));
        }
        else
        {
            selectedView->setCenter(DRect(start_x, m_obContentSize.height/2, selectSize.width, selectSize.height));
            this->addSubview(selectedView);
        }
        
        this->reloadComponent(1,i, true);
        
        start_x += tableWidth;
    }

    for (auto& tableView : m_tableViews)
    {
        tableView->reloadData();
    }
}


void CAPickerView::reloadComponent(unsigned int _row,unsigned int component, bool bReloadData)
{
    // reload component
    unsigned int row = 0;
    if (m_obNumberOfRowsInComponentCallback)
    {
        row = m_obNumberOfRowsInComponentCallback(component);
    }
    else if (m_dataSource)
    {
        row = m_dataSource->numberOfRowsInComponent(this, component);
    }
    
    int head = m_displayRow[component]/2;
    int foot = m_displayRow[component]/2;
    if (row <= m_displayRow[component])
    {
        row += (head + foot);
        m_componentsIndex[component].resize(row);
        for (int i=0; i<row; i++)
        {
            if (i < head)
            {
                m_componentsIndex[component][i] = -1;
            }
            else if (i >= row - foot)
            {
                m_componentsIndex[component][i] = -1;
            }
            else
            {
                m_componentsIndex[component][i] = i - head;
            }
        }
    }
    else
    {
        int cycle = 3;
        m_componentsIndex[component].resize(row*3);
        while (cycle--)
        {
            for (int i=0; i<row; i++)
            {
                m_componentsIndex[component][i + cycle*row] = i;
            }
        }
    }
        
    // reset selected index
    this->selectRow(_row, component, false);
    
    if (bReloadData)
    {
        // reload table view
		CATableView* view = m_tableViews.at(component);

        view->reloadData();   
        view->stopDeaccelerateScroll();
    }
}

CAView* CAPickerView::viewForRowInComponent(int component, int row, DSize size)
{
    int index = m_componentsIndex[component][row];
    if (index == -1)
    {
        return NULL;
    }
    
    CAView* view = nullptr;
    if (m_obViewForRowCallback)
    {
        view = m_obViewForRowCallback(index, component);
    }
    else if (m_dataSource)
    {
        view = m_dataSource->viewForRow(this, index, component);
    }
    
    if (view == nullptr)
    {
        std::string title;
        if (m_obTitleForRowCallback)
        {
            title = m_obTitleForRowCallback(row, component);
        }
        else if (m_dataSource)
        {
            title = m_dataSource->titleForRow(this, index, component);
        }
        
        if (!title.empty())
        {
            DRect rect = DRect(0, 0, size.width, size.height);
            CALabel* label = CALabel::createWithFrame(rect);
            label->setText(title);
			label->setColor(m_fontColorNormal);
            label->setFontSize(m_fontSizeNormal);
            label->setVerticalTextAlignmet(CAVerticalTextAlignment::Center);
            label->setTextAlignment(CATextAlignment::Center);
            
            return label;
        }
    
    }
    return view;
}

CATableViewCell* CAPickerView::tableCellAtIndex(CATableView* table, const DSize& cellSize, unsigned int section, unsigned int row)
{
    CATableViewCell* cell = table->dequeueReusableCellWithIdentifier("CrossApp");
    if (cell == NULL)
    {
        cell = CATableViewCell::create("CrossApp");
        cell->setBackgroundView(nullptr);
    }
    else
    {
        cell->removeSubviewByTag(100);
    }
    
    size_t component = m_tableViews.getIndex(table);
    
    CAView* view = viewForRowInComponent((unsigned int)component, row, cellSize);
    if (view)
    {
        view->setTag(100);
        cell->addSubview(view);
    }
    
    return cell;
}

unsigned int CAPickerView::numberOfRowsInSection(CATableView *table, unsigned int section)
{
    size_t component = m_tableViews.getIndex(table);
    return (unsigned int)m_componentsIndex[component].size();
}

unsigned int CAPickerView::tableViewHeightForRowAtIndexPath(CATableView* table, unsigned int section, unsigned int row)
{
    unsigned int component = (unsigned int)m_tableViews.getIndex(table);
    unsigned int rowHeight = 0;
    if (m_obHeightForComponentCallback)
    {
        rowHeight = m_obHeightForComponentCallback(component);
    }
    else if (m_dataSource)
    {
        rowHeight = m_dataSource->rowHeightForComponent(this, component);
    }
    
    return rowHeight;
}

void CAPickerView::selectRow(unsigned int row, unsigned int component, bool animated)
{
    CATableView* tableView = m_tableViews.at(component);
    if (tableView)
    {
        unsigned int maxRow = 0;
        if (m_obNumberOfRowsInComponentCallback)
        {
            maxRow = m_obNumberOfRowsInComponentCallback(component);
        }
        else if (m_dataSource)
        {
            maxRow = m_dataSource->numberOfRowsInComponent(this, component);
        }
        unsigned int rowHeight = 0;
        if (m_obHeightForComponentCallback)
        {
            rowHeight = m_obHeightForComponentCallback(component);
        }
        else if (m_dataSource)
        {
            rowHeight = m_dataSource->rowHeightForComponent(this, component);
        }
        
        if (row < maxRow)
        {
            //DPoint offset = DPointZero;
            DPoint offset;
            if (maxRow <= m_displayRow[component])
            {
                m_selected[component] = row + m_displayRow[component]/2;
                offset.y = row * rowHeight;
                tableView->setContentOffset(offset, false);
            }
            else
            {
                m_selected[component] = maxRow + row;
                offset.y = m_selected[component] * rowHeight + rowHeight/2 - tableView->getBounds().size.height/2;
                tableView->setContentOffset(offset, false);
            }
        }
    }
}

int CAPickerView::selectedRowInComponent(unsigned int component)
{
    return m_selected[component];
}

void CAPickerView::setBackgroundColor(const CAColor4B& color)
{
	this->setColor(color);
}

void CAPickerView::visitEve()
{
    for (int i = 0; i < m_tableViews.size(); i++)
    {
        // cycle data
        CATableView* tableView = (CATableView*)m_tableViews.at(i);
        DPoint offset = tableView->getContentOffset();
        unsigned int component = (unsigned int)m_tableViews.getIndex(tableView);
        
        unsigned int row = 0;
        if (m_obNumberOfRowsInComponentCallback)
        {
            row = m_obNumberOfRowsInComponentCallback(component);
        }
        else if (m_dataSource)
        {
            row = m_dataSource->numberOfRowsInComponent(this, component);
        }
        
        unsigned int rowHeight = 0;
        if (m_obHeightForComponentCallback)
        {
            rowHeight = m_obHeightForComponentCallback(i);
        }
        else if (m_dataSource)
        {
            rowHeight = m_dataSource->rowHeightForComponent(this, i);
        }
        
        if (row > m_displayRow[component])
        {
            if (offset.y < 0)
            {
                offset.y += row * 2 * rowHeight;
                tableView->setContentOffset(offset, false);
            }
            else if (offset.y > rowHeight * m_componentsIndex[component].size() - tableView->getFrame().size.height)
            {
                offset.y -= row * 2 * rowHeight;
                tableView->setContentOffset(offset, false);
            }
        }
        
        
        // set opacity
        int offset_y = abs((int)offset.y);
        int remainder = offset_y % rowHeight;
        int index = offset_y / rowHeight;
        
        if (remainder >= rowHeight * 0.5)
        {
            index++;
        }
        
        for (int i=index-1; i<index + m_displayRow[component] + 2; i++)
        {
            CATableViewCell* cell = tableView->cellForRowAtIndexPath(0, i);
            if (cell)
            {
                float y = cell->getFrameOrigin().y - offset_y + cell->getFrame().size.height/2;
                float mid = tableView->getFrame().size.height/2;
                float length = fabs(mid - y);
                float op = (fabs(length - mid))/mid;
                op = powf(op, 2);
                op = MAX(op, 0.1f);
                cell->setAlpha(op);
            }
        }
        
        // fixed position in the middle
        if (!tableView->isDecelerating() && !tableView->isTracking())
        {
            if (remainder > rowHeight/2)
            {
                index++;
                offset.y = index * rowHeight;
                tableView->setContentOffset(offset, true);
            }
            else if (remainder > 0)
            {
                offset.y = index * rowHeight;
                tableView->setContentOffset(offset, true);
            }
            else
            {
                // set selected when stop scrolling.
                
                int selected = index + m_displayRow[component]/2;
                
                if (m_selected[component] != selected)
                {
                    m_selected[component] = selected;
                    if (m_obDidSelectRowCallback)
                    {
                        m_obDidSelectRowCallback(m_componentsIndex[component][m_selected[component]], component);
                    }
                    else if (m_delegate)
                    {
                        m_delegate->didSelectRow(this, m_componentsIndex[component][m_selected[component]], component);
                    }
                }
            }
            
        }
    }
    
    CAView::visitEve();
}

NS_CC_END
