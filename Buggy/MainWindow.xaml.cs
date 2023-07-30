using System;
using System.Collections.Generic;
using System.Linq;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Security.Principal;
using System.Threading;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;

namespace Buggy
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        [DllImport("kernel32.dll")]
        public static extern IntPtr OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll")]
        public static extern bool ReadProcessMemory(int hProcess, UInt64 lpBaseAddress, byte[] lpBuffer, int dwSize, ref int lpNumberOfBytesRead);


        private const int PROCESS_ALL_ACCESS = (0x1F0FFF);
        private Process bugdomProcess;
        private IntPtr processHandle = IntPtr.Zero;
        private Byte[] myBuffer = new Byte[4];
        private bool foundProcess = false;
        private bool foundPointers = false;

        private string filePathToPointers = string.Empty;

        // uint32
        private UInt32 ptrManagerStringState = 0;
        private UInt32 ptrPaused = 0;
        private UInt32 ptrOtherState = 0;
        private UInt32 ptrPositionState = 0;
        private UInt32 ptrPlaybackManagerString = 0;

        private bool FindProcess()
        {
            if(Process.GetProcessesByName("Ato").Length > 0)
            {
                if(!foundProcess)
                {
                    try
                    {
                        bugdomProcess = Process.GetProcessesByName("Ato")[0];
                        processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, bugdomProcess.Id);
                        filePathToPointers = bugdomProcess.MainModule.FileName.Replace("Ato.exe", "pointer_ref.txt");

                        foundProcess = true;

                    }
                    catch(Exception e)
                    {
                        System.Diagnostics.Trace.WriteLine("Couldn't open process: {0}", e.ToString());
                    }
                }

                if(foundProcess)
                {
                    
                    try
                    {

                        Thread.Sleep(3000);
                        string ptrData = System.IO.File.ReadAllText(filePathToPointers);
                        string[] ptrs = ptrData.Split(',');

                        UInt32.TryParse(ptrs[0], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out ptrManagerStringState);
                        System.Diagnostics.Trace.WriteLine(string.Format("stringStatePtr: {0:X}", ptrManagerStringState));
                        UInt32.TryParse(ptrs[1], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out ptrPaused);

         
                        foundPointers = true;
                    }
                    catch(Exception what)
                    {
                        System.Diagnostics.Trace.WriteLine("Couldn't read pointers: {0}", what.ToString());
                    }

                }

                return (foundProcess);
            }

            return false;
        }
        public MainWindow()
        {
            InitializeComponent();

            CompositionTarget.Rendering += PreRenderUpdate;
        }


        private void Redo_FindProcess(object sender, EventArgs e)
        {
            this.foundProcess = false;
            this.foundPointers = false;

            lblmyinfo.Content = "Waiting for Ato . .";

            FindProcess();
        }

        private void PreRenderUpdate(object sender, EventArgs e)
        {
            if(Process.GetProcessesByName("Ato").Length == 0)
            {
                processHandle = IntPtr.Zero;
                
                this.foundProcess = false;
                this.foundPointers = false;
                lblmyinfo.Content = "Waiting for Ato . .";
            }

            if(foundProcess && foundPointers)
            {
                UpdateLabelInfo(processHandle);
            }
            else
            {
                foundProcess = false;
                foundPointers = false;
                FindProcess();

            }
        }

        private void UpdateLabelInfo(IntPtr procHandle)
        {
            if (procHandle == IntPtr.Zero)
                return;

            bool isPaused = BitConverter.ToBoolean(ReadValue(ptrPaused, 1), 0);


            string playbackString = string.Empty;
            string playbackOtherString = string.Empty;
            string playerStateString = string.Empty;
            string playerPositionString = string.Empty;
            string playbackManagerString = string.Empty;

            playbackString = System.Text.Encoding.UTF8.GetString(ReadValue64(ptrManagerStringState, 2000));
            //playbackOtherString = System.Text.Encoding.UTF8.GetString(ReadValue64(ptrOtherState, 2000));
            //playerPositionString = System.Text.Encoding.UTF8.GetString(ReadValue64(ptrPositionState, 2000));
            //playbackManagerString = System.Text.Encoding.UTF8.GetString(ReadValue64(ptrPlaybackManagerString, 2000));


            lblmyinfo.Content = "Paused:" + isPaused + "\n" + playbackString;
        }

        private void Window_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
                this.DragMove();
        }

        private void MenuItemExit_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void Img_MouseEnter(object sender, MouseEventArgs e)
        {
            imgMoveWindow.Opacity = 1;
        }

        private void Img_MouseLeave(object sender, MouseEventArgs e)
        {
            imgMoveWindow.Opacity = 0.05;
        }
        private void ImgMoveWindow_MouseEnter(object sender, MouseEventArgs e)
        {

        }

        private Byte[] ReadValue(UInt32 address, int size)
        {
            Byte[] myValBuffer = new Byte[size];
            int bytesRead = 0;

            if (!ReadProcessMemory((int)processHandle, address, myValBuffer, myValBuffer.Length, ref bytesRead))
                System.Diagnostics.Debug.WriteLine("Couldn't read from {0:X} address", address);

            return myValBuffer;
        }

        private Byte[] ReadValue64(UInt64 address, int size)
        {
            Byte[] myValBuffer = new Byte[size];
            int bytesRead = 0;

            if (!ReadProcessMemory((int)processHandle, address, myValBuffer, myValBuffer.Length, ref bytesRead))
                System.Diagnostics.Debug.WriteLine("Couldn't read from {0:X} address", address);

            return myValBuffer;
        }


        private UInt64 ReadAddress64(UInt64 address, UInt32[] offsets)
        {
            int bytesRead = 0;

            if (offsets != null)
            {
                for (int i = 0; i < offsets.Length; i++)
                {
                    ReadProcessMemory((int)processHandle, address + offsets[i], myBuffer, myBuffer.Length, ref bytesRead);
                    UInt64.TryParse(ByteArrayToString(myBuffer), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out address);
                    address = (UInt32)BitConverter.ToInt64(myBuffer, 0);
                }
                return address;
            }

            ReadProcessMemory((int)processHandle, address, myBuffer, myBuffer.Length, ref bytesRead);
            UInt64.TryParse(ByteArrayToString(myBuffer), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out address);
            address = (UInt64)BitConverter.ToInt64(myBuffer, 0);

            return address;
        }
        private UInt32 ReadAddress(UInt32 address, UInt32[] offsets)
        {
            int bytesRead = 0;

            if (offsets != null)
            {
                for (int i = 0; i < offsets.Length; i++)
                {
                    ReadProcessMemory((int)processHandle, address + offsets[i], myBuffer, myBuffer.Length, ref bytesRead);
                    UInt32.TryParse(ByteArrayToString(myBuffer), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out address);
                    address = (UInt32)BitConverter.ToInt32(myBuffer, 0);
                }
                return address;
            }

            ReadProcessMemory((int)processHandle, address, myBuffer, myBuffer.Length, ref bytesRead);
            UInt32.TryParse(ByteArrayToString(myBuffer), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out address);
            address = (UInt32)BitConverter.ToInt32(myBuffer, 0);

            return address;
        }

        public static string ByteArrayToString(byte[] ba)
        {
            return BitConverter.ToString(ba).Replace("-", "");
        }
    }
}
