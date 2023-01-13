--!NEEDS:ui/uipanel.lua
AUI=AUI or {}
AUI.Main=Core.class(UI.Panel,function(lib) end)

local function sceneTreeMapper(item) 
	if item.assetName then 
		return AUI.TreeItem.new(item)
	else
		local sub={}
		for i=1,item:getNumChildren() do
			sub[i]=item:getChildAt(i)
			sub[i]=sub[i].model or sub[i]
		end
		return AUI.TreeItem.new(item),sub
	end
end

function AUI.Main:init()
	local Template={
		class="UI.Panel", Color="colBackground", 
		layoutModel={ 
			rowWeights={0,1,0}, columnWeights={0,1,0},
		},
		layout={fill=1},
		children={
			-- Top bar
			{ class= UI.Panel, layoutModel={ columnWeights={1}, }, layout={fill=1, gridx=0,gridy=0, gridwidth=3, insets="aui_main.szSpacing"}, children={
				{ class="UI.Toolbox",
					data={ { "btNew","file-plus"},{ "btLoad","file"},{ "btSave","content-save"},{ "btSaveAs","content-save-edit"}, {"btExport", "export"} },
					dataMapper=function(d)
						local fd=UI.Image.new()
						UI.Behavior.Button.new(fd)
						fd:setLayoutConstraints({width="2is", height="2is"})
						fd:setImage("|R|aui/icons/"..d[2]..".png")
						fd.name=d[1]
						return fd
					end,
					Header={ class="UI.Image", Image="|R|aui/icons/file-cabinet.png", layout={ width="2is", height="2is" } },
					Direction=true, --Horizontal
					layout={ anchorx=0 },
				},
				{ class="UI.Label", Text="Gideros Scene Editor", layout={ },  },
				}
			},
			-- Left side
			{ class= UI.Panel, layoutModel=UI.Layout.Vertical, layout={fill=1, gridx=0,gridy=1, width="aui_asshelf.szAll"}, children={
				{ class="UI.Panel", LocalStyle="aui_main.styLabel", layoutModel={ rowWeights={1}, columnWeights={1,0}}, layout={ fill=1 },  
				  children={
					{ class="UI.Label", Text="Library", layout={ fill=1 },  },
					{ class="UI.Image", Image="aui_main.icPlus", layout={ gridx=1, fill=0, width="1em", height="1em" }, 
					name="btLibraryAdd", behavior=UI.Behavior.Button, },
				  }
				},
				{ class=UI.Viewport, layout={fill=1,weighty=1 },
					Scrollbar={UI.Viewport.SCROLLBAR.AUTO,UI.Viewport.SCROLLBAR.AUTO},
					Content={
						class=AUI.AssetLibrary, name="assetLibrary", layout={ anchorx=0, anchory=0, fill=Sprite.LAYOUT_FILL_HORIZONTAL},
					}					
				},
				}
			},
			-- Center
			{ class=AUI.Editor, layout={gridx=1,gridy=1, fill=1}, name="editor",
			},
			-- Right side
			{ class="UI.Splitpane", Vertical=true, layout={fill=1, gridx=2,gridy=1, width="aui_asshelf.szAll"},	
				--Directory Tree
				Tabs={.1,.3,.8},
				OpenDirection=nil,
				First={ class= UI.Panel, layoutModel=UI.Layout.Vertical, layout={fill=1}, children={
					{ class="UI.Label", Style="aui_main.styLabel", Text="Scene tree", layout={ fill=1 },  },
					{ class=UI.Viewport, layout={fill=1, weighty=1, },
						Scrollbar={UI.Viewport.SCROLLBAR.AUTO,UI.Viewport.SCROLLBAR.AUTO},					
						Content={
							class=UI.Tree, name="sceneTree",
							layout={fill=1},
							selection=UI.Selection.SINGLE,
						}
					},
				}},
				Second={ class= UI.Panel, layoutModel=UI.Layout.Vertical, layout={fill=1}, children={
					{ class="UI.Label", Style="aui_main.styLabel", Text="Object properties", layout={ fill=1 },  },
					{ class=UI.Viewport, layout={fill=1, weighty=1, },
						Scrollbar={UI.Viewport.SCROLLBAR.AUTO,UI.Viewport.SCROLLBAR.AUTO},					
						Content={
							class=AUI.PropertyList, name="objProps",
							layout={fill=1},
						}
					},
				}},
			},
			-- Bottom bar
			{ class="UI.Panel", layoutModel=UI.Layout.Horizontal, layout={ gridx=0, gridwidth=3, gridy=2},
				children={
					{ class=UI.Panel,layout={weightx=1} }, --Filler
				},
			}
		}
	}
	UI.BuilderSelf(Template,self)
	

	self.allAssets=AssetShelf.loadAll()
	--assetLib:import("I:/3D/Assets/Packs/KayKit-Dungeon")

	self.assetLibrary:updateLibrary(self.allAssets)
	
	self:newProject()
	
end

function AUI.Main:newProject()
	self.projectFile=nil
	self.editor:clearSceneData()
	self:initProject()
end

function AUI.Main:loadProject(file)
	local loadGroup
	function loadGroup(g,p)
		p.name=g.name
		p.tag=g.tag
		if g.transform then
			if g.transform.position then
				p:setPosition(g.transform.position[1],g.transform.position[2],g.transform.position[3])
			end
			if g.transform.rotation then
				p:setRotationX(g.transform.rotation[1])
				p:setRotationY(g.transform.rotation[2])
				p:setRotation(g.transform.rotation[3])
			end
			if g.transform.scale then
				p:setScale(g.transform.scale[1],g.transform.scale[2],g.transform.scale[3])
			end
		end
		for _,s in ipairs(g.children) do
			if s.asset then
				alib=nil
				for _,al in ipairs(self.allAssets) do 
					if al.name==s.asset.lib then alib=al end
				end
				if alib then
					local m=self.editor:addModel({ name=s.asset.name, lib=alib},p)
					m.name=s.name
					m.tag=s.tag
					if s.transform then
						local t=m:getTransform()
						if s.transform.position then
							t:setPosition(s.transform.position[1],s.transform.position[2],s.transform.position[3])
						end
						if s.transform.rotation then
							t:setRotationX(s.transform.rotation[1])
							t:setRotationY(s.transform.rotation[2])
							t:setRotationZ(s.transform.rotation[3])
						end
						if s.transform.scale then
							t:setScale(s.transform.scale[1],s.transform.scale[2],s.transform.scale[3])
						end
					end
					if s.physics then
						local s=s.physics
						local pspec={ shape=s.shapetype }
						if s.transform then
							local t=Matrix.new()
							pspec.transform=t
							if s.transform.position then
								t:setPosition(s.transform.position[1],s.transform.position[2],s.transform.position[3])
							end
							if s.transform.rotation then
								t:setRotationX(s.transform.rotation[1])
								t:setRotationY(s.transform.rotation[2])
								t:setRotationZ(s.transform.rotation[3])
							end
							if s.transform.scale then
								t:setScale(s.transform.scale[1],s.transform.scale[2],s.transform.scale[3])
							end
						end
						m:makeBody(pspec)
						if s.bodytype then m.body.bodytype=s.bodytype end
					else
						m:makeBody(pspec)
					end
					m:update(s.physics)
				end
			else
				local m=self.editor:addGroup(p)
				loadGroup(s,m)
			end
		end
	end
	local project=Files.loadJson(file)
	self.editor:clearSceneData()	
	if project then
		self.projectFile=file
		loadGroup(project.scene,self.editor:getSceneData())
	end
	self:initProject()
end

function AUI.Main:saveProject(file)
	local outputGroup
	function outputGroup(g)
		local sub={}
		for i=1,g:getNumChildren() do
			local it=g:getChildAt(i)
			if it.model then
				it=it.model
				local t=it:getTransform()
				local tp=it.body.fixtureTransform
				sub[i]={
					asset={ name=it.assetName, lib=it.assetLib.name, },
					name=it.name,
					tag=it.tag,
					transform={
						position={ t:getX(), t:getY(), t:getZ() },
						rotation={ t:getRotationX(), t:getRotationY(), t:getRotationZ() },
						scale= { t:getScaleX(), t:getScaleY(), t:getScaleZ() },
					},
					physics={
						bodytype=it.body.bodytype,
						shapetype=it.body.shapetype,
						transform={
							position={ tp:getX(), tp:getY(), tp:getZ() },
							rotation={ tp:getRotationX(), tp:getRotationY(), tp:getRotationZ() },
							scale= { tp:getScaleX(), tp:getScaleY(), tp:getScaleZ() },
						},
					}
				}
			else
				sub[i]=outputGroup(it)
			end
		end
		return { name=g.name, tag=g.tag, children=sub,
			transform={
				position={ g:getX(), g:getY(), g:getZ() },
				rotation={ g:getRotationX(), g:getRotationY(), g:getRotation() },
				scale= { g:getScaleX(), g:getScaleY(), g:getScaleZ() },
			}
		}
	end
	local scene=self.editor:getSceneData()
	local project={ scene=outputGroup(scene) }
	
	self.projectFile=file or self.projectFile
	Files.saveJson(self.projectFile,project)
end

function AUI.Main:exportAssets(folder)
	local gatherAssets
	local assets={}
	function gatherAssets(g)
		local sub={}
		for i=1,g:getNumChildren() do
			local it=g:getChildAt(i)
			if it.model then
				it=it.model
				local atag=it.assetLib.name.."/"..it.assetName
				assets[atag]={name=it.assetName, lib=it.assetLib}
			else
				gatherAssets(it)
			end
		end
	end
	gatherAssets(self.editor:getSceneData())
	for _,n in pairs(assets) do
		n.lib:exportAsset(n.name, folder, {})
	end
end

function AUI.Main:initProject()
	self.editor:selectModel(nil)
	self.sceneTree:setData({ self.editor:getSceneData() },sceneTreeMapper)
	self.sceneTree:setExpanded(self.editor:getSceneData(),true)
end

function AUI.Main:onSceneChanged(w)
	self.sceneTree:setData({ w:getSceneData() },sceneTreeMapper)
end

function AUI.Main:onSceneModelSelected(w,s)
	UI.Selection.select(self.sceneTree, { s })
	self.objProps:setObject(if s and s.getPropertyList then s else nil)
end
function AUI.Main:onSceneModelUpdated(w,s)
	if s==self.editor.selection then
		self.objProps:setObject(if s and s.getPropertyList then s else nil)
	end
end
function AUI.Main:onSelectionChange(w,s)
	if w==self.sceneTree then
		if s then _,s=next(s) end
		self.editor:selectModel(s)
	end
	self.objProps:setObject(if s and s.getPropertyList then s else nil)
end

function AUI.Main:onWidgetAction(w)
	if w and w.name=="btSave" then
		if not self.projectFile then
			file=application:get("saveFileDialog","Save scene as",nil,"Scene (*.gscn)")
			if file and file~="" then 
				self.projectFile=file
			end
		end
		if self.projectFile then
			self:saveProject()
		end
		return true
	elseif w and w.name=="btSaveAs" then
		local file=application:get("saveFileDialog","Save scene as",nil,"Scene (*.gscn)")
		if file and file~="" then
			self:saveProject(file)
		end
		return true
	elseif w and w.name=="btLoad" then
		local file=application:get("openFileDialog","Choose a scene file",nil,"Scene (*.gscn)")
		if file and file~="" then 
			self:loadProject(file)
		end
		return true
	elseif w and w.name=="btNew" then
		self:newProject()
		return true
	elseif w and w.name=="btExport" then
		local file=application:get("openDirectoryDialog","Export assets to",nil)
		if file and file~="" then
			self:exportAssets(file)
		end
		return true
	elseif w==self.btLibraryAdd then
		local file=application:get("openDirectoryDialog","Import Library",nil)
		local name=file
		while name:find("/") do name=name:sub(name:find("/")+1) end
		local assetLib=AssetShelf.new(name)
		assetLib:import(file)
		table.insert(self.allAssets,assetLib)
		self.assetLibrary:updateLibrary(self.allAssets)
	elseif w==self.btInfo then
		local dialog=UI.Dialog.confirm("Do you agree","Yes!","No :(")
		local scw=screen:getWidth()
		dialog:setAnchorPosition(scw,0)
		UI.ModalScreen.showDialog(screen,dialog,function(res,dialog,cleanup)
			UI.Animation:animate(dialog,"pop",UI.Animation.AnchorMove,3000,{x=-scw,onStop=cleanup})
			return true
		end,"colShadow",100)
		UI.Animation:animate(dialog,"pop",UI.Animation.AnchorMove,1000,{ easing={type="bounce",way="in"}})
	end
end

function AUI.Main:onAssetShelfDelete(w,name)
	local dialog=UI.Dialog.confirm("This will remove asset collection \e[color=colHighlight]"..name.."\e[color] from your library.","I confirm","Cancel")
	dialog:setLayoutConstraints({fill=Sprite.LAYOUT_FILL_NONE, width="20em"})
	UI.ModalScreen.showDialog(UI.Screen.getScreen(w),dialog,function(res,dialog,cleanup)
		if res then
			local idx
			for k,alib in ipairs(self.allAssets) do
				if alib.name==name then idx=k end
			end
			if idx then
				local al=table.remove(self.allAssets,idx)
				al:delete()
			end
			self.assetLibrary:updateLibrary(self.allAssets)
		end
		cleanup()
		return true
	end,"colShadow",100)
end
