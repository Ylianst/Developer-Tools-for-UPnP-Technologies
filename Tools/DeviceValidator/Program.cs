/*   
Copyright 2006 - 2010 Intel Corporation

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

using System;
using System.IO;
using System.Windows.Forms;
using System.Collections.Generic;

namespace UPnPValidator
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            Application.ThreadException += new System.Threading.ThreadExceptionEventHandler(ExceptionSink);
            Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException, true);
            AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(UnhandledExceptionEventSink);

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }

        public static void ExceptionSink(object sender, System.Threading.ThreadExceptionEventArgs args)
        {
            OpenSource.UPnP.AutoUpdate.ReportCrash(Application.ProductName, args.Exception.ToString());
            OpenSource.Utilities.InstanceTracker.GenerateExceptionFile("Exceptions-Validator.txt", args.Exception.ToString(), OpenSource.Utilities.InstanceTracker.VersionString);
        }

        public static void UnhandledExceptionEventSink(object sender, UnhandledExceptionEventArgs args)
        {
            OpenSource.UPnP.AutoUpdate.ReportCrash(Application.ProductName, ((Exception)args.ExceptionObject).ToString());
            OpenSource.Utilities.InstanceTracker.GenerateExceptionFile("Exceptions-Validator.txt", ((Exception)args.ExceptionObject).ToString(), OpenSource.Utilities.InstanceTracker.VersionString);
        }
    }
}
