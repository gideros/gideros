<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
    <title>Reference</title>
    <link rel="stylesheet" href="luadoc.css" type="text/css" />
	<!--meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/-->
</head>

<body>
<div id="container">

<div id="main">

<div id="content">

<h1><%=class.name%></h1>

<p>

<%=class.description%>

</p>



<h2>Functions</h2>
<table class="function_list">

<% for _,f in ipairs(class.functions) do
	local params = {}
	for _,p in ipairs(f.parameters) do
		params[#params + 1] = p.name
	end
%>

	<tr>
	<td class="name" nowrap><a href="#<%=f.name%>"><%=f.name%></a>&nbsp;(<%=table.concat(params, ", ")%>)</td>
	<td class="summary">Returns a table with the file attributes corresponding to <code>filepath</code> (or <code>nil</code> followed by an error message in case of error).</td>
	</tr>

<% end %>

	
</table>






<br/>
<br/>



<h2><a name="functions"></a>Functions</h2>
<dl class="function">



<dt><a name="attributes"></a><strong>attributes</strong>&nbsp;(filepath, aname)</dt>
<dd>
Returns a table with the file attributes corresponding to <code>filepath</code> (or <code>nil</code> followed by an error message in case of error). If the second optional argument is given, then only the value of the named attribute is returned (this use is equivalent to <code>lfs.attributes(filepath).aname</code>, but the table is not created and only one attribute is retrieved from the O.S.). The attributes are described as follows; attribute <code>mode</code> is a string, all the others are numbers, and the time related attributes use the same time reference of <a href="http://www.lua.org/manual/5.0/manual.html#5.7"><code>os.time</code></a>.


<h3>Parameters</h3>
<ul>
	
	<li>
	  filepath: 
	</li>
	
	<li>
	  aname: 
	</li>
	
</ul>






</dd>




<dt><a name="chdir"></a><strong>chdir</strong>&nbsp;(path)</dt>
<dd>
Changes the current working directory to the given <code>path</code>.<br /> Returns <code>true</code> in case of success or <code>nil</code> plus an error string.


<h3>Parameters</h3>
<ul>
	
	<li>
	  path: 
	</li>
	
</ul>






</dd>




<dt><a name="currentdir"></a><strong>currentdir</strong>&nbsp;()</dt>
<dd>
Returns a string with the current working directory or <code>nil</code> plus an error string.







</dd>




<dt><a name="dir"></a><strong>dir</strong>&nbsp;(path)</dt>
<dd>
Lua iterator over the entries of a given directory. Each time the iterator is called it returns a string with an entry of the directory; <code>nil</code> is returned when there is no more entries. Raises an error if <code>path</code> is not a directory.


<h3>Parameters</h3>
<ul>
	
	<li>
	  path: 
	</li>
	
</ul>






</dd>




<dt><a name="lock"></a><strong>lock</strong>&nbsp;(filehandle, mode, start, length)</dt>
<dd>
Locks a file or a part of it. This function works on <em>open files</em>; the file handle should be specified as the first argument. The string <code>mode</code> could be either <code>r</code> (for a read/shared lock) or <code>w</code> (for a write/exclusive lock). The optional arguments <code>start</code> and <code>length</code> can be used to specify a starting point and its length; both should be numbers.<br /> Returns <code>true</code> if the operation was successful; in case of error, it returns <code>nil</code> plus an error string.


<h3>Parameters</h3>
<ul>
	
	<li>
	  filehandle: 
	</li>
	
	<li>
	  mode: 
	</li>
	
	<li>
	  start: 
	</li>
	
	<li>
	  length: 
	</li>
	
</ul>






</dd>




<dt><a name="mkdir"></a><strong>mkdir</strong>&nbsp;(dirname)</dt>
<dd>
Creates a new directory. The argument is the name of the new directory.<br /> Returns <code>true</code> if the operation was successful; in case of error, it returns <code>nil</code> plus an error string.


<h3>Parameters</h3>
<ul>
	
	<li>
	  dirname: 
	</li>
	
</ul>






</dd>




<dt><a name="rmdir"></a><strong>rmdir</strong>&nbsp;(dirname)</dt>
<dd>
Removes an existing directory. The argument is the name of the directory.<br /> Returns <code>true</code> if the operation was successful; in case of error, it returns <code>nil</code> plus an error string.


<h3>Parameters</h3>
<ul>
	
	<li>
	  dirname: 
	</li>
	
</ul>






</dd>




<dt><a name="touch"></a><strong>touch</strong>&nbsp;(filepath, atime, mtime)</dt>
<dd>
Set access and modification times of a file. This function is a bind to <code>utime</code> function. The first argument is the filename, the second argument (<code>atime</code>) is the access time, and the third argument (<code>mtime</code>) is the modification time. Both times are provided in seconds (which should be generated with Lua standard function <code>os.date</code>). If the modification time is omitted, the access time provided is used; if both times are omitted, the current time is used.<br /> Returns <code>true</code> if the operation was successful; in case of error, it returns <code>nil</code> plus an error string.


<h3>Parameters</h3>
<ul>
	
	<li>
	  filepath: 
	</li>
	
	<li>
	  atime: 
	</li>
	
	<li>
	  mtime: 
	</li>
	
</ul>






</dd>




<dt><a name="unlock"></a><strong>unlock</strong>&nbsp;(filehandle, start, length)</dt>
<dd>
Unlocks a file or a part of it. This function works on <em>open files</em>; the file handle should be specified as the first argument. The optional arguments <code>start</code> and <code>length</code> can be used to specify a starting point and its length; both should be numbers.<br /> Returns <code>true</code> if the operation was successful; in case of error, it returns <code>nil</code> plus an error string.


<h3>Parameters</h3>
<ul>
	
	<li>
	  filehandle: 
	</li>
	
	<li>
	  start: 
	</li>
	
	<li>
	  length: 
	</li>
	
</ul>






</dd>


</dl>






</div> <!-- id="content" -->

</div> <!-- id="main" -->


</div> <!-- id="container" -->	
</body>
</html>
