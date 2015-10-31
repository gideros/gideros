Module.gui_displayDialog=function(gid,title,message,text,cancelButton,Button1,Button2,isSecure,callback){
    var container = document.createElement("div");
    container.setAttribute("id", "giddialog_"+gid);
    container.className = "gid_dialog_container";
    
/*    document.getElementById("canvas").onmouseup = function(event){event.preventDefault()};
    document.getElementById("canvas").onmousedown = function(event){event.preventDefault()};
    document.getElementById("canvas").onclick = function(event){event.preventDefault()};
 */   
    var overlay = document.createElement("div");
    overlay.className = "gid_dialog_overlay";
    container.appendChild(overlay);
    
    var dialog_wrapper = document.createElement("div");
    dialog_wrapper.className = "gid_dialog_wrapper";
    container.appendChild(dialog_wrapper);
    
    var dialog = document.createElement("div");
    dialog.className = "gid_dialog";
    dialog_wrapper.appendChild(dialog);
    
    var titleEl = document.createElement("h1");
    titleEl.className = "gid_dialog_title";
    titleEl.innerHTML = title || "";
    dialog.appendChild(titleEl);
    
    var messageEl = document.createElement("p");
    messageEl.className = "gid_dialog_message";
    messageEl.innerHTML = message || "";
    dialog.appendChild(messageEl);
    
    if(text){
        var input = document.createElement("input");
        input.className = "gid_dialog_input";
        input.value = text || "";
        if(isSecure)
            input.setAttribute("type", "password");
        else
            input.setAttribute("type", "text");
        dialog.appendChild(input);
    }
    
    var buttons = document.createElement("div");
    buttons.className = "gid_dialog_buttons";
    dialog.appendChild(buttons);
    
    var button_handler = function(){
        if(callback){
            callback(gid, parseInt(this.getAttribute("id").replace("index_", "")), this.value, (input) ? input.value : null);
        }
        document.body.removeChild(container);
    };
    
    var cancel = document.createElement("input");
    cancel.className = "gid_dialog_button gid_dialog_btn_cancel";
    cancel.setAttribute('id', "index_0");
	cancel.setAttribute("type", "button");
	cancel.value = cancelButton || "Cancel";
    cancel.onclick = button_handler;
	buttons.appendChild(cancel);
    
    if(Button1){
        var button1 = document.createElement("input");
        button1.className = "gid_dialog_button gid_dialog_btn_1";
        button1.setAttribute('id', "index_1");
        button1.setAttribute("type", "button");
        button1.value = Button1 || "";
        button1.onclick = button_handler;
        buttons.appendChild(button1);
    }
    
    if(Button2){
        var button2 = document.createElement("input");
        button2.className = "gid_dialog_button gid_dialog_btn_2";
        button2.setAttribute('id', "index_2");
        button2.setAttribute("type", "button");
        button2.value = Button2 || "";
        button2.onclick = button_handler;
        buttons.appendChild(button2);
    }
    document.body.appendChild(container);
}
Module.gui_hideDialog=function(gid){
    if(document.getElementById("giddialog_"+gid)){
        document.body.removeChild(document.getElementById("giddialog_"+gid));
    }
}
