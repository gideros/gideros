<html>

<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<link href="css/bundle_common.css" media="screen" rel="stylesheet" type="text/css" />
<link href="css/bundle_github.css" media="screen" rel="stylesheet" type="text/css" />
<link href="css/extra.css" media="screen" rel="stylesheet" type="text/css" />
<link href="prettify/prettify.css" type="text/css" rel="stylesheet" />
<script type="text/javascript" src="prettify/prettify.js"></script>
<script type="text/javascript" src="prettify/lang-lua.js"></script>
<script type="text/javascript" src="jquery-1.4.4.min.js"></script>

<style>
</style>

<script>

$(document).ready(function(){

	$("code").addClass("prettyprint").addClass("lang-lua");

<%if doc.toc then%>
	$(function(){
		$(".uplist").children("a.closed").click(function(){
			if($(this).hasClass("closed"))
			{
				$(this).addClass("opened").removeClass("closed");
				$(this).parent().find("ul").slideDown();
			}
			else
			{
				$(this).addClass("closed").removeClass("opened");
				$(this).parent().find("ul").slideUp();
			}
		});
	});
<%end%>
});


</script>

</head>


<body onload="prettyPrint()">

<div class="site">
<div id="readme" class="blob">
<div class="wikistyle">

<%if doc.toc then%>
<div id="contents">
<h1>Table of Contents</h1>
<ul>
<%for i=1,#toc do%>
	<li class='uplist'><a href="#<%= toc[i].className%>" class='closed'><strong><%= toc[i].className%><% if toc[i].baseClass then %> > <%= toc[i].baseClass%><%end%></strong></a>
		<ul>
			<%for j=1,#toc[i] do%>
			<li><a href="#<%=toc[i][j]%>"><%=toc[i][j]%></a></li>
			<%end%>
		</ul>
	</li>
<%end%>
</ul>
<p>&nbsp;</p>
</div>
<%end%>

<%for i=1,#doc do %>

	<% if doc[i].type == "function" then %>
		<a name="<%= doc[i].functionName%>"></a><h2><%= doc[i].functionFullName..(doc[i].isObsolete and "<span style='color:#a00000;'> (obsolete)</span>" or "")%></h2>
		<%= discount(doc[i].functionDescription) %>

		<% if #doc[i].parameters ~= 0 then %>
			<h4>Parameters:</h4>
			<p>
			<%for j=1,#doc[i].parameters do%>

				&nbsp;&nbsp;<strong><%= doc[i].parameters[j].name %>:</strong>&nbsp;&nbsp;<%= doc[i].parameters[j].description %></br>

				<%--[[%>
				<div>
				<div class="parameter_name"><%= doc[i].parameters[j].name %>:</div><div class="parameter_desc"><%= doc[i].parameters[j].description %></div>
				<div style="clear:both"></div>
				</div>
				<%--]]%>
			<%end%>
			</p>
		<%end%>

		<% if doc[i].returns ~= "" then %>
			<h4>Returns:</h4>
			<%= discount(doc[i].returns) %>
		<%end%>


		<% if doc[i].seeAlso ~= "" then %>
			<h4>See Also:</h4>
			<%= discount(doc[i].seeAlso) %>
		<%end%>

		<% if doc[i].example ~= "" then %>
			<h4>Example:</h4>
			<%= discount(doc[i].example) %>
		<%end%>


	<% elseif doc[i].type == "class" then %>
		<a name="<%= doc[i].className%>"></a><h1><%= doc[i].className%><% if doc[i].baseClass then %> > <%= doc[i].baseClass%><%end%></h1>
		<%= discount(doc[i].classDescription) %>
		<% if doc[i].example ~= "" then %>
			<h4>Example:</h4>
			<%= discount(doc[i].example) %>
		<%end%>
	<% elseif doc[i].type == "info" then %>
		<%= discount(doc[i].info) %>
	<%end%>

<%end%>

</div>
</div>
</div>

</body>
</html>
