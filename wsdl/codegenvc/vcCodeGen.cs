// $Header: c:/cvs/pocketsoap/wsdl/codegenvc/vcCodeGen.cs,v 1.2 2004/06/09 05:41:55 simon Exp $

using System;

namespace PocketSOAP.WSDL
{
	public class vcCodeGen : wsdlCodeGen.ICodeGenTarget	
	{
		public vcCodeGen()
		{
		}
	
		private string m_directory ;
		private string m_wsdlUrl;
		private wsdlParser.definitions m_wsdl;
		private MAPLib.MapCollection m_context;
		private vcProject m_project;
		private ProxyWriter m_currentproxy;

		public void Initialize(string genDirectory, string wsdlUrl, wsdlParser.definitions wsdlDef, MAPLib.MapCollection ctxParams)
		{
			m_directory = genDirectory;
			m_wsdlUrl = wsdlUrl;
			m_wsdl = wsdlDef;
			m_context = ctxParams;
			string projName ;
			try 
			{
				projName = (string)m_context["project"];
			} 
			catch (Exception)
			{
				projName = m_wsdl.Name;
			}
			if (projName.Length == 0)
				projName = "SoapProxy";

			CodeGenContext.Current.ProjectName = projName;
			CodeGenContext.Current.Directory = m_directory;
			CodeGenContext.Current.WsdlUrl = m_wsdlUrl;

			m_project = new vcProject(m_directory, projName);
		}

		public void Finalize()
		{
			// generate project file
			m_project.Finalize();
			m_wsdl = null ;
			m_context = null;
		}

		public void StartProxy(wsdlParser.port port, wsdlParser.binding binding, wsdlParser.portType portType)
		{
			m_currentproxy = new ProxyWriter(m_wsdlUrl, m_wsdl, port, binding, portType, m_project);
		}
	
		public void Operation(string opName)
		{
			m_currentproxy.Operation(opName);
		}
	
		public void FinalizeProxy()
		{
			m_currentproxy.FinalizeProxy();
		}
	
	}
}
