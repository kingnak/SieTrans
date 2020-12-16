function Component() 
{ 
    // default constructor 
} 
 
Component.prototype.createOperations = function() 
{ 
    component.createOperations(); 
	component.addOperation("CreateShortcut", "@TargetDir@/SieTrans.exe", "@StartMenuDir@/SieTrans.lnk", "workingDirectory=@TargetDir@"); 
} 
