
/*
This JS file is the global-script for the DocGen Wizard.  It creates a few COM 
objects, initializes some collections, and provides a few global-scope helper 
functions.
*/

//
// Global variables
//
var idlMunger; // an instance of GenX.IDLMunger
var xmlMunger; // an instance of Microsoft.XMLDOM

var fso; // an instance of Scripting.FileSystemObject
var fsoLog; // the DocGen.log file

var clcCoClassItems; // collection of coclass items
var clcInterfaceItems; // collection of interface items
var clcMethodItems; // collection of method/property items

var clcAltTOCs; // collection of alternative toc names

var nErrors = 0; // number of errors encountered
var nWarnings = 0; // number of warnings encountered


//
// This class definiton serves to relate an idlMunger object to its parent 
// object, which is useful for establishing links, unique filenames, and such.
// All file-output collections will contain objects of this class.
//
function CollectionItem(idlSelf, idlParent)
  {
  this.idlSelf = idlSelf;
  this.idlParent = idlParent;
  }


//
// The ubiquitous "for each" language construct/template/pattern, as applied 
// for OLE collections
//
function for_each(coll, func) {
  for (var i=0; i < coll.Count; ++i) {
    func(coll.Item(i)); 
    }
  }


//
// The ubiquitous "for each" language construct/template/pattern, as applied 
// for XMLDOM nodelists
//
function for_each_xmlnode(coll, func) {
  for (var i=0; i < coll.length; ++i) {
    func(coll.item(i)); 
    }
  }


//
// A different spin on the old "for each" idea -- for formatting lists of 
// items, it's often convenient to know when you're dealing with the first 
// item, or the last one
//
function for_each_x(coll, func) {
  for (var i=0; i < coll.Count; ++i) {
    func(coll.Item(i),i,coll.Count);
    }
  }


//
// Helper function to create missing xml elements; used in "auto-gen" mode
//
function selectOrCreateChildElement(xmlThis,sChildElemName,sThisObjectName,sChildObjectName,sDefaultChildElemText,bWarningMode)
  {
  if (xmlThis == null) return null;
  if (sChildElemName == null) return null;
  // all other parameters are optional
  
  var sXPathQuery = ("./"+sChildElemName+((sChildObjectName==null)?(""):("[@name='"+sChildObjectName+"']")));
  var xmlChild = xmlThis.selectSingleNode(sXPathQuery);
  if (xmlChild == null)
    {
    // Log the error... but only when we can place the blame in a constructive way
    if (sThisObjectName != null)
      {
      fsoLog.WriteLine(((bWarningMode)?("Warning: "):("Error: "))+"incomplete documentation for '"+sThisObjectName+"': \""+sXPathQuery+"\"\nFor a complete list of offending items, search the resulting HTML output for \"///todo\".\n");
    
      if (bWarningMode) 
        nWarnings++;
      else
        nErrors++; // we'll abort later, but keep churning for now
      }

    xmlChild = xmlThis.ownerDocument.createElement(sChildElemName);
    
    if (sChildObjectName != null)
      xmlChild.setAttribute("name",sChildObjectName);
    
    if (sDefaultChildElemText != null)
      {
      xmlChild.text = "///todo: "+sDefaultChildElemText;
      }
    
    xmlThis.appendChild(xmlChild);
    }

  return xmlChild;
  }


//
// Helper function to htmlize a string
//
function makeSafeForHtml(s)
  {
  while (s.search("<") >= 0)
    s = s.replace("<","&lt;");
  while (s.search(">") >= 0)
    s = s.replace(">","&gt;");
  while (s.search("\"") >= 0)
    s = s.replace("\"","&quot;");
  return s;
  }


//
// Wizard event handlers
//
function wizard_onPreUI()
  {
  // Instantiate the fso, for logging
  try {
    fso = new ActiveXObject("Scripting.FileSystemObject");
    fsoLog = fso.CreateTextFile("DocGen.log",true,false); // overwrite=yes, unicode=no
    }
  catch (e) {
    wizard.Alert("Could not cocreate Scripting.FileSystemObject...\n\n"+e.number+"\n"+e.message);
    wizard.SetAbort();
    return;
    }

  // Instantiate the mungers
  try {
    idlMunger = new ActiveXObject("GenX.IDLMunger");
    }
  catch (e) {
    fsoLog.WriteLine("Could not cocreate GenX.IDLMunger...\n\n"+e.number+"\n"+e.message+"\n");
    wizard.SetAbort();
    return;
    }

  try {
    xmlMunger = new ActiveXObject("Microsoft.XMLDOM");
    }
  catch (e) {
    fsoLog.WriteLine("Could not cocreate Microsoft.XMLDOM...\n\n"+e.number+"\n"+e.message+"\n");
    wizard.SetAbort();
    return;
    }

  return;
  }

function wizard_onPreProduceFiles()
  {
  // Open the source files
  try {
    var b = xmlMunger.load(input.xmlFile); // does not throw!
  
    if (!b && input.bAutoGen)
      b = xmlMunger.loadXML("<Library/>");
    
    if (!b)
      throw new Error(0x80070002,"load failed: '"+input.xmlFile+"'");
    }
  catch (e) {
    fsoLog.WriteLine("Could not load the XML file...\n\n"+e.number+"\n"+e.message+"\n");
    wizard.SetAbort();
    return;
    }

  try {
    idlMunger.Open(input.idlFile);
    }
  catch (e) {
    fsoLog.WriteLine("Could not open the IDL file...\n\n"+e.number+"\n"+e.message+"\n");
    wizard.SetAbort();
    return;
    }

  // Prepare to build the collection of interfaces (both inside the lib, and out)
  clcInterfaceItems = new ActiveXObject("GenX.Collection");

  var idlInterfaces = idlMunger.Interfaces;
  var idlLibInterfaces = idlMunger.Library.Interfaces;
  
  // Add global-scope interfaces
  for (var i=0; i < idlInterfaces.Count; ++i)
    { 
    var bHidden = false; // presumed innocent until proven guilty
    
    for (var j=0; j < idlInterfaces(i).Attributes.Count; ++j)
      if (idlInterfaces(i).Attributes(j).Name == "hidden")
        bHidden = true;

    if (!bHidden)
      {
      var itm = new CollectionItem(idlInterfaces(i),idlMunger.Library);
      clcInterfaceItems.Add(itm); 
      }
    }
  
  // Add library-scope interfaces, unless a duplicate (global-scope) already exists
  for (var i=0; i < idlLibInterfaces.Count; ++i)
    {
    var bAlreadyThere = false; // presumed innocent until proven guilty
    
    for (var j=0; j < clcInterfaceItems.Count; ++j)
      if (clcInterfaceItems.Item(j).idlSelf.Name == idlLibInterfaces(i).Name) 
        bAlreadyThere = true;

    var bHidden = false; // always presumed innocent until proven guilty
    
    for (var j=0; j < idlLibInterfaces(i).Attributes.Count; ++j)
      if (idlLibInterfaces(i).Attributes(j).Name == "hidden")
        bHidden = true;

    if (!bHidden && !bAlreadyThere) 
      {
      var itm = new CollectionItem(idlLibInterfaces(i),idlMunger.Library);
      clcInterfaceItems.Add(itm);
      }
    }
  
  // Prepare to build the collection of coclasses
  clcCoClassItems = new ActiveXObject("GenX.Collection");

  var idlCoClasses = idlMunger.Library.CoClasses;
  
  // Add all coclass declarations, except the [noncreatable] ones
  for (var i=0; i < idlCoClasses.Count; ++i)
    {
    var bHidden = false; // presumed innocent until proven guilty
    
    for (var j=0; j < idlCoClasses(i).Attributes.Count; ++j)
      if (idlCoClasses(i).Attributes(j).Name == "hidden" ||
          idlCoClasses(i).Attributes(j).Name == "noncreatable") 
        bHidden = true;

    if (!bHidden) 
      {
      var itm = new CollectionItem(idlCoClasses(i),idlMunger.Library);
      clcCoClassItems.Add(itm);
      }
    }
  
  // Prepare to build the collection of methods
  clcMethodItems = new ActiveXObject("GenX.Collection");
  
  // For each interface...
  for (var i=0; i < clcInterfaceItems.Count; ++i)
    {
    // Add all methods, except for duplicates (ie: propget vs propput)
    for (var j=0; j < clcInterfaceItems.Item(i).idlSelf.Methods.Count; ++j)
      {
      var itm = new CollectionItem(clcInterfaceItems.Item(i).idlSelf.Methods(j),clcInterfaceItems.Item(i).idlSelf);

      var bAlreadyThere = false; // presumed innocent until proven guilty

      for (var k=0; k < clcMethodItems.Count; ++k)
        if (clcMethodItems.Item(k).idlSelf.Name == itm.idlSelf.Name && 
            clcMethodItems.Item(k).idlParent.Name == itm.idlParent.Name)
          bAlreadyThere = true;

      var bHidden = false; // again, presumed innocent until proven guilty
    
      for (var k=0; k < clcInterfaceItems.Item(i).idlSelf.Methods(j).Attributes.Count; ++k)
        if (clcInterfaceItems.Item(i).idlSelf.Methods(j).Attributes(k).Name == "hidden")
          bHidden = true;

      if (!bHidden && !bAlreadyThere)
        clcMethodItems.Add(itm);
      }
    }
    
  // Prepare to build the collection of alternative toc names
  clcAltTOCs = new ActiveXObject("GenX.Collection");
    clcAltTOCs.Add(input.projectName); // a placeholder for the primary toc
  
  // Add each alt toc name...
  var xmlNodes = xmlMunger.documentElement.selectNodes("./AltTOCs/AltTOC/@name");

  for (var xml=xmlNodes.nextNode(); xml != null; xml=xmlNodes.nextNode())
    clcAltTOCs.Add(xml.text);

  return;
  }

function wizard_onPostProduceFiles()
  {
  try {
    if (input.bAutoGen)
      {
      xmlMunger.save(input.xmlFile);
      if (!wizard.quiet)
        wizard.Alert("'"+input.xmlFile+"' was updated successfully!","DocGen");
      }
    }
  catch (e) {
    nErrors++;
    var s = "Error: could not save the XML file...\n\t"+e.message+" ("+e.number+")\n\n";
    fsoLog.WriteLine(s);
    if (!wizard.quiet)
      wizard.Alert(s);
    }
  
  if (nErrors > 0) 
    {
    if (!wizard.quiet)
      wizard.Alert("One or more errors occurred...\nPlease see the \"DocGen.log\" file for details.");
    wizard.SetAbort();
    }
  
  fsoLog.WriteLine(input.projectDirectory+"\\"+input.projectName+".hhp - "+nErrors+" error(s), "+nWarnings+" warning(s)");
  
  fsoLog.Close();
  
  return;
  }
