using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Input;

namespace csharp_test_client
{
    public partial class GameForm : Form
    {
        ClientSimpleTcp GameNetwork = new ClientSimpleTcp();

        bool IsNetworkThreadRunning = false;
        bool IsBackGroundProcessRunning = false;

        System.Threading.Thread GameNetworkReadThread = null;
        System.Threading.Thread GameNetworkSendThread = null;

        PacketBufferManager PacketBuffer = new PacketBufferManager();
        Queue<PacketData> RecvPacketQueue = new Queue<PacketData>();
        Queue<byte[]> SendPacketQueue = new Queue<byte[]>();

        System.Windows.Threading.DispatcherTimer dispatcherUITimer;
        Dictionary<PACKET_ID, Action<byte[]>> GamePacketFuncDic = new Dictionary<PACKET_ID, Action<byte[]>>();

        readonly Int64 key;
        
        public GameForm(Int64 keyInput)
        {
            key = keyInput;
            InitializeComponent();
        }

        private void GameForm_Load(object sender, EventArgs e)
        {
            PacketBuffer.Init((8096 * 10), PacketDef.PACKET_HEADER_SIZE, 1024);
            string address = "127.0.0.1";
            int port = 3335;
            bool isConnected = false;
            for (UInt16 i = 0; i < 10; i++)
            {
                if (GameNetwork.Connect(address, port))
                {
                    isConnected = true;
                    break;
                }
                else
                {
                    System.Threading.Thread.Sleep(500);
                }
            }
            if (isConnected != true)
            {
                this.Close();
            }

            IsNetworkThreadRunning = true;
            GameNetworkReadThread = new System.Threading.Thread(this.NetworkReadProcess);
            GameNetworkReadThread.Start();
            GameNetworkSendThread = new System.Threading.Thread(this.NetworkSendProcess);
            GameNetworkSendThread.Start();

            IsBackGroundProcessRunning = true;
            //dispatcherUITimer = new System.Windows.Threading.DispatcherTimer();
            //dispatcherUITimer.Tick += new EventHandler(BackGroundProcess);
            //dispatcherUITimer.Interval = new TimeSpan(0, 0, 0, 0, 100);
            //dispatcherUITimer.Start();
            var requestPkt = new GameEnterReqPacket();
            requestPkt.setKey(key);
            PostSendPacket(PACKET_ID.GAME_ENTER_REQUEST, requestPkt.ToBytes());
            //DevLog.Write($"방 입장 요청:  {textBoxRoomNumber.Text} 번");

            BackgroundWorker worker = new BackgroundWorker();
            worker.DoWork += new DoWorkEventHandler(worker_DoWork);
            worker.RunWorkerAsync();
            //btnDisconnect.Enabled = false;

            SetPacketHandler();
            DevLog.Write("프로그램 시작 !!!", LOG_LEVEL.INFO);
        }

        private void GameForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            IsNetworkThreadRunning = false;
            IsBackGroundProcessRunning = false;
            GameNetwork.Close();
        }

        void NetworkReadProcess()
        {
            const Int16 PacketHeaderSize = PacketDef.PACKET_HEADER_SIZE;

            while (IsNetworkThreadRunning)
            {
                if (GameNetwork.IsConnected() == false)
                {
                    System.Threading.Thread.Sleep(1);
                    continue;
                }

                var recvData = GameNetwork.Receive();

                if (recvData != null)
                {
                    PacketBuffer.Write(recvData.Item2, 0, recvData.Item1);

                    while (true)
                    {
                        var data = PacketBuffer.Read();
                        if (data.Count < 1)
                        {
                            break;
                        }

                        var packet = new PacketData();
                        packet.DataSize = (short)(data.Count - PacketHeaderSize);
                        packet.PacketID = BitConverter.ToInt16(data.Array, data.Offset + 2);
                        packet.Type = (SByte)data.Array[(data.Offset + 4)];
                        packet.BodyData = new byte[packet.DataSize];
                        Buffer.BlockCopy(data.Array, (data.Offset + PacketHeaderSize), packet.BodyData, 0, (data.Count - PacketHeaderSize));
                        lock (((System.Collections.ICollection)RecvPacketQueue).SyncRoot)
                        {
                            RecvPacketQueue.Enqueue(packet);
                        }
                    }

                    DevLog.Write($"받은 데이터 크기: {recvData.Item1}", LOG_LEVEL.INFO);
                }
                else
                {
                    GameNetwork.Close();
                    SetDisconnectd();
                    DevLog.Write("서버와 접속 종료 !!!", LOG_LEVEL.INFO);
                }
            }
        }

        void NetworkSendProcess()
        {
            while (IsNetworkThreadRunning)
            {
                System.Threading.Thread.Sleep(1);

                if (GameNetwork.IsConnected() == false)
                {
                    continue;
                }

                lock (((System.Collections.ICollection)SendPacketQueue).SyncRoot)
                {
                    if (SendPacketQueue.Count > 0)
                    {
                        var packet = SendPacketQueue.Dequeue();
                        GameNetwork.Send(packet);
                    }
                }
            }
        }


        void worker_DoWork(object sender, DoWorkEventArgs e)
        {
            //ProcessLog();
            while (IsNetworkThreadRunning)
            {
                try
                {
                    var packet = new PacketData();

                    lock (((System.Collections.ICollection)RecvPacketQueue).SyncRoot)
                    {
                        if (RecvPacketQueue.Count() > 0)
                        {
                            packet = RecvPacketQueue.Dequeue();
                        }
                    }

                    if (packet.PacketID != 0)
                    {
                        GamePacketProcess(packet);
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(string.Format("ReadPacketQueueProcess. error:{0}", ex.Message));
                }
            }
        }

        public void SetDisconnectd()
        {
            //if (btnConnect.Enabled == false)
            //{
            //    btnConnect.Enabled = true;
            //    btnDisconnect.Enabled = false;
            //}

            //SendPacketQueue.Clear();

            //listBoxRoomChatMsg.Items.Clear();
            //listBoxRoomUserList.Items.Clear();

            //labelStatus.Text = "서버 접속이 끊어짐";
            SendPacketQueue.Clear();
        }

        public void PostSendPacket(PACKET_ID packetID, byte[] bodyData)
        {
            if (GameNetwork.IsConnected() == false)
            {
                DevLog.Write("서버 연결이 되어 있지 않습니다", LOG_LEVEL.ERROR);
                return;
            }

            Int16 bodyDataSize = 0;
            if (bodyData != null)
            {
                bodyDataSize = (Int16)bodyData.Length;
            }
            var packetSize = bodyDataSize + PacketDef.PACKET_HEADER_SIZE;

            List<byte> dataSource = new List<byte>();
            dataSource.AddRange(BitConverter.GetBytes((UInt16)packetSize));
            dataSource.AddRange(BitConverter.GetBytes((UInt16)packetID));
            dataSource.AddRange(new byte[] { (byte)0 });

            if (bodyData != null)
            {
                dataSource.AddRange(bodyData);
            }

            SendPacketQueue.Enqueue(dataSource.ToArray());
        }


        //private void ProcessLog()
        //{
        //    // 너무 이 작업만 할 수 없으므로 일정 작업 이상을 하면 일단 패스한다.
        //    int logWorkCount = 0;

        //    while (IsBackGroundProcessRunning)
        //    {
        //        System.Threading.Thread.Sleep(1);

        //        string msg;

        //        if (DevLog.GetLog(out msg))
        //        {
        //            ++logWorkCount;

        //            //if (listBoxLog.Items.Count > 512)
        //            //{
        //            //    listBoxLog.Items.Clear();
        //            //}

        //            //listBoxLog.Items.Add(msg);
        //            //listBoxLog.SelectedIndex = listBoxLog.Items.Count - 1;
        //        }
        //        else
        //        {
        //            break;
        //        }

        //        if (logWorkCount > 8)
        //        {
        //            break;
        //        }
        //    }
        //}

        void SetPacketHandler()
        {
            //GamePacketFuncDic.Add(PACKET_ID.DEV_ECHO, PacketProcess_DevEcho);
            GamePacketFuncDic.Add(PACKET_ID.GAME_LAPSE_NOTIFY, PacketProcess_GameNotify);
            GamePacketFuncDic.Add(PACKET_ID.GAME_RESULT_NOTIFY, PacektProcess_GameResultNtf);
            
        }

        void GamePacketProcess(PacketData packet)
        {
            var packetType = (PACKET_ID)packet.PacketID;
            //DevLog.Write("Packet Error:  PacketID:{packet.PacketID.ToString()},  Error: {(ERROR_CODE)packet.Result}");
            //DevLog.Write("RawPacket: " + packet.PacketID.ToString() + ", " + PacketDump.Bytes(packet.BodyData));

            if (GamePacketFuncDic.ContainsKey(packetType))
            {
                GamePacketFuncDic[packetType](packet.BodyData);
            }
            else
            {
                DevLog.Write("Unknown Packet Id: " + packet.PacketID.ToString());
            }
        }

        void PacketProcess_GameNotify(byte[] bodyData)
        {
            var responsePkt = new GameLapseNtfPacket();
            responsePkt.FromBytes(bodyData);

            //AddRoomChatMessageList(responsePkt.UserID, responsePkt.Message);
            //this.Close();
            this.notifyDataList.Invoke(new MethodInvoker(
                        delegate { this.notifyDataList.ClearSelected(); }
                        ));
            this.notifyDataList.Invoke(new MethodInvoker(
                        delegate { this.notifyDataList.Text = Convert.ToString(responsePkt.LapsValue); }
                        ));
            
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void notifyDataList_selectDataexcange(object sender, EventArgs e)
        {

        }

        private void WinButton_Click(object sender, EventArgs e)
        {
            var requestPkt = new GameControlRequstPacket();
            requestPkt.setKey(key);
            requestPkt.setButton(true);
            PostSendPacket(PACKET_ID.GAME_CONTROL_REQ, requestPkt.ToBytes());
        }

        void PacektProcess_GameResultNtf(byte[] bodyData)
        {
            var notifyPkt = new GameResultNotifyPacket();
            notifyPkt.FromBytes(bodyData);

            if (notifyPkt.result == true)
            {
                this.WinButton.Invoke( new MethodInvoker(
                        delegate { this.WinButton.Text = "YOU WIN!"; }
                        ));
            }
            else
            {
                this.WinButton.Invoke(new MethodInvoker(
                        delegate { this.WinButton.Text = "loser www"; }
                        ));
            }
            System.Threading.Thread.Sleep(5000);
            this.Invoke(new MethodInvoker(
                        delegate { this.Close(); }
                        ));
        }
    }

   
}
