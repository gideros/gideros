--!NEEDS:uiinit.lua
--Event args. Handler is always given target(self) and source before any optional args
UI.Event={
  TextChange    	={ "data" },          			--For editable widgets    : uitextfield
  TextValid   		={ "data" },          			--For editable widgets    : uitextfield
  FocusChange   	={ "data","focused","event" },  --For editable widgets    : uitextfield --event={TAG="...",reason="...}
  FocusArea   		={ "x","y","w","h" },     		--For focusable widgets   : sent when specified source widget area should be made visible, sent by UI.Focus:area() and handled by viewport
  WidgetChange  	={ "data","newState" },     	--For clickable widgets   : uibreadcrumb, uicheckbox, uiradio,uitabbedpane [without state]: uicombobox, uicalendar [with looped instead of state]: uispinner [with ratio instead of data]: uisplitpane, uislider [with page after ratio:] uiscrollbar
  WidgetChanging	={ "booleean" },				--For slidable/draggable/repeatable changing items: uislider
  WidgetExpand  	={ "data","expanded" },     	--For expandable widgets  : uitree, uiaccordion
  WidgetAction  	={ },               			--From button Behavior
  WidgetLongAction	={ },							--From LongClick Behavior
  WidgetLingerStart	={ "x", "y" },					--From Linger Behavior
  WidgetLingerEnd	={ "x", "y" },					--From Linger Behavior
  SelectionChange 	={ "selection" }, 				--From Selection engine
  ColumnMove		={ "oldIndex", "newIndex" }, 	--From uitable
  ColumnResize		={ "columnIndex", "newSize" }, 	--From uitable
  RangeChange		={ "rangeWidth", "rangeHeight"},--From uiviewport
  ViewChange		={ "scrollX", "scrollY", "inProgress" },--From uiviewport
  ItemMove			={ "data", "insertPoint", "subData" }, --From uitree	--From uiaccordion		
  DndDrop			={ "source", "dndData", "targetData", "insertPoint"},	--From uiaccordion			: dndData is the dnd package dropped, targetData is the object on which drop occured, if any
}

function UI.dispatchEvent(source,eventName,...)
  assert(source,"Event source isn't a Sprite") --NO assert(source.getParent,"Event source isn't a Sprite") --else not dispatch WidgetExpand tree
  local ehnd="on"..eventName
  local tgt=source --NO :getParent() --else not dispatch WidgetExpand tree
  while tgt do
    --print("uievent UI.dispatchEvent",tgt,tgt:getClass(),ehnd,"handler?",tgt[ehnd])
    if tgt[ehnd] then
		local stop,ret=tgt[ehnd](tgt,source,...)
		if stop then return ret end --handler must return something to stop while
	end
    tgt=tgt:getParent()
  end
end