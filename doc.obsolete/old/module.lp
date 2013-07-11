<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
    <title><%=module_doc.name%> Reference</title>
    <link rel="stylesheet" href="luadoc.css" type="text/css" />
	<!--meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/-->
</head>

<body>
<div id="container">

<div id="product">
	<div id="product_logo"></div>
	<div id="product_name"><big><b></b></big></div>
	<div id="product_description"></div>
</div> <!-- id="product" -->

<div id="main">

<% --[[ %>
<div id="navigation">

</div><!-- id="navigation" -->
<% --]] %>


<div id="content">

<h1>Class <code><%=module_doc.name%></code></h1>

<p><%=module_doc.description%></p>
<%if module_doc.author then%>
<p><b><%= #module_doc.author>1 and "Authors" or "Author" %>:</b>
<table class="authors_list">
<%for _, author in ipairs(module_doc.author) do%>
	<tr><td class="name"><%= author %></td></tr>
<%end%>
</table>
</p>
<%end%>
<%if module_doc.copyright then%>
<p>Copyright&copy; <%=module_doc.copyright%></p>
<%end%>
<%if module_doc.release then%>
<p><small><b>Release:</b> <%=module_doc.release%></small></p>
<%end%>


<%if #module_doc.properties > 0 then%>
<h2>Properties</h2>
<table class="function_list">
<%for _, prop_name in ipairs(module_doc.properties) do
  local prop_data = module_doc.properties[prop_name]
  local type = prop_data.type
  local link = links[type] and ('<a href="' .. links[type] .. '">' .. type .. "</a>") or type
  %>
	<tr>
	<td class="name" nowrap><%=link%>&nbsp;<a href="#<%=prop_name%>"><%=prop_name%></a></td>
	<td class="summary"><%=prop_data.read_only and "[read-only] " or ""%><%=module_doc.properties[prop_name].summary%></td>
	</tr>  
<%end%>
</table>
<%end%>


<%if module_doc.fields and #module_doc.fields > 0 then%>
<h2>Fields</h2>
<table class="function_list">
<%for _, field_name in ipairs(module_doc.fields) do
  local field_data = module_doc.fields[field_name]
  local type = field_data.type
  local link = links[type] and ('<a href="' .. links[type] .. '">' .. type .. "</a>") or type
%>
  <tr>
    <td class="name" nowrap><%=link%>&nbsp;<a href="#<%=field_name%>"><%=field_name%></a>&nbsp;=&nbsp;<%=field_data.value%></td>
    <td class="summary"><%=field_data.summary%></td>
  </tr>
<%end%>
</table>
<%end%>

<%if #module_doc.functions > 0 then%>
<h2>Functions</h2>
<table class="function_list">
<%for _, func_name in ipairs(module_doc.functions) do
  local func_data = module_doc.functions[func_name]
  
  local params = {}
  for _,param in ipairs(func_data.param) do
    local type = func_data.param[param].type
    local link = links[type] and ('<a href="' .. links[type] .. '">' .. type .. "</a>") or type
    params[#params + 1] = link .. " " .. param
  end
  %>
	<tr>
	<td class="name" nowrap><%=func_data.private and "local " or ""%><a href="#<%=func_name%>"><%=func_name%></a>&nbsp;(<%=table.concat(params, ", ")%>)</td>
	<td class="summary"><%=module_doc.functions[func_name].summary%></td>
	</tr>
<%end%>
</table>
<%end%>


<%if #module_doc.tables > 0 then%>
<h2>Tables</h2>
<table class="table_list">
<%for _, tab_name in ipairs(module_doc.tables) do%>
	<tr>
	<td class="name" nowrap><a href="#<%=tab_name%>"><%=tab_name%></a></td>
	<td class="summary"><%=module_doc.tables[tab_name].summary%></td>
	</tr>
<%end%>
</table>
<%end%>


<br/>
<br/>


<%if #module_doc.properties > 0 then%>
<h2><a name="properties"></a>Properties</h2>
<dl class="function">
<%for _, prop_name in ipairs(module_doc.properties) do%>

<%
local prop=module_doc.properties[prop_name]
local prop_data = module_doc.properties[prop_name]
local link = links[prop_data.type] and ('<a href="' .. links[prop_data.type] .. '">' .. prop_data.type .. "</a>") or prop_data.type
%>
<dt><a name="<%=prop_name%>"></a><%=link%>&nbsp;<strong><%=prop_name%></strong></dt>
<dd>
<%=prop.read_only and "[read-only] " or ""%><%=prop.description or ""%>
</dd>

<%end%>
</dl>
<%end%>

<%if module_doc.fields and #module_doc.fields > 0 then%>
<h2><a name="fields"></a>Fields</h2>
<dl class="function">
<%for _, field_name in ipairs(module_doc.fields) do%>

<%
local field=module_doc.fields[field_name]
local field_data = module_doc.fields[field_name]
local link = links[field_data.type] and ('<a href="' .. links[field_data.type] .. '">' .. field_data.type .. "</a>") or field_data.type
%>

<dt><a name="<%=field_name%>"></a><%=link%>&nbsp;<strong><%=field_name%></strong></dt>
<dd>
<%=field.description or ""%>
</dd>

<%end%>
</dl>
<%end%>



<%if #module_doc.functions > 0 then%>
<h2><a name="functions"></a>Functions</h2>
<dl class="function">
<%for _, func_name in ipairs(module_doc.functions) do%>

<%
local func=module_doc.functions[func_name]
%>
<%
  local func_data=module_doc.functions[func_name]
  local params = {}
  for _,param in ipairs(func_data.param) do
    local type = func_data.param[param].type
    local link = links[type] and ('<a href="' .. links[type] .. '">' .. type .. "</a>") or type
    params[#params + 1] = link .. " " .. param
  end
%>

<dt><%=func.private and "local " or ""%><a name="<%=func_name%>"></a><strong><%=func_name%></strong>&nbsp;(<%=table.concat(params, ", ")%>)</dt>
<dd>
<%=func.description or ""%>

<%if type(func.param) == "table" and #func.param > 0 then%>
<h3>Parameters</h3>
<ul>
	<%for p = 1, #func.param do%>
	<li>
	  <%=func.param[p]%>: <%=func.param[func.param[p]].description or ""%>
	</li>
	<%end%>
</ul>
<%end%>


<%if type(func.usage) == "string" then%>
<h3>Usage:</h3>
<%=func.usage%>
<%elseif type(func.usage) == "table" then%>
<h3>Usage</h3>
<ul>
	<%for _, usage in ipairs(func.usage) do%>
	<li><%= usage %>
	<%end%>
</ul>
<%end%>

<%if type(func.ret) == "string" then%>
<h3>Return value:</h3>
<%=func.ret%>
<%elseif type(func.ret) == "table" then%>
<h3>Return values:</h3>
<ol>
	<%for _, ret in ipairs(func.ret) do%>
	<li><%= ret %>
	<%end%>
</ol>
<%end%>

<%if type(func.see) == "string" then %>
<h3>See also:</h3>
	<a href="<%=func.see%>"><%=func.see%></a>
<%elseif type(func.see) == "table" and #func.see > 0 then %>
<h3>See also:</h3>
<ul>
	<%for i = 1, #func.see do%>
	<li><a href="<%=luadoc.doclet.html.symbol_link(func.see[i], doc, module_doc, file_doc, from)%>">
		<%=func.see[i]%>
	</a>
	<%end%>
</ul>
<%end%>
</dd>




<%end%>
</dl>
<%end%>

<%if #module_doc.tables > 0 then%>
<h2><a name="tables"></a>Tables</h2>
<dl class="table">
<%for _, tab_name in ipairs(module_doc.tables) do%>




<% local tab=module_doc.tables[tab_name] %>

<dt><a name="<%=tab_name%>"></a><strong><%=tab_name%></strong></dt>
<dd><%=tab.description%>

<%if type(tab.field) == "table" and #tab.field > 0 then%>
<em>Fields</em>
<ul>
	<%for p = 1, #tab.field do%>
	<li>
	  <%=tab.field[p]%>: <%=tab.field[tab.field[p]] or ""%>
	</li>
	<%end%>
</ul>
<%end%>

</dd>






<%end%>
</dl>
<%end%>



</div> <!-- id="content" -->

</div> <!-- id="main" -->

<% --[[ %>
<div id="about">
	<p><a href="http://validator.w3.org/check?uri=referer"><img src="http://www.w3.org/Icons/valid-xhtml10" alt="Valid XHTML 1.0!" height="31" width="88" /></a></p>
</div> <!-- id="about" -->
<% --]] %>

</div> <!-- id="container" -->	
</body>
</html>
