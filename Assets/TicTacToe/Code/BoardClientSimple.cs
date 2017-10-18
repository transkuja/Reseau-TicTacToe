using UnityEngine;
using System.Collections;
using System;
using System.IO;
using System.Threading;
using System.Collections.Generic;
using System.Net.Sockets;

public class BoardClientSimple : MonoBehaviour {

    Board m_Board = new Board();

    int m_MyId;

    public Board Board { get { return m_Board; } }

    public string hostIp = "127.0.0.1";
    public int hostPort = 52250;

    public bool isSocketReady = false;

    TcpClient clientSocket;
    NetworkStream stream;
    StreamWriter writer;
    StreamReader reader;

    // Use this for initialization
    void Start () {
        Board.Init();
        setupSocket();
        string msg = "hello?";
        writeSocket(msg);
        Debug.Log("[CLIENT] -> " + msg);

    }

	// Update is called once per frame
	void Update () {
        string received = readSocket();

        if (received != "")
        {
            int j;
            // The message received is the id
            if (Int32.TryParse(received, out j))
            {
                m_MyId = j;
                if (m_MyId == 0)
                {
                    Board.Start();
                    writeSocket(((int)Board.GetCurrentTurnPlayer()).ToString());
                }
            }
            // The message received is the new board state
            else
                Debug.Log("[SERVER]" + received);
        }
    }

    //try to initiate connection
    public void setupSocket()
    {
        try
        {
            clientSocket = new TcpClient(hostIp, 52250);
            stream = clientSocket.GetStream();
            writer = new StreamWriter(stream);
            reader = new StreamReader(stream);
            isSocketReady = true;
        }
        catch (Exception e)
        {
            Debug.Log("Socket error:" + e);
        }
    }

    //send message to server
    public void writeSocket(string theLine)
    {
        if (!isSocketReady)
            return;
        String tmpString = theLine + "\r\n";
        writer.Write(tmpString);
        writer.Flush();
    }

    //read message from server
    public string readSocket()
    {
        String result = "";
        if (stream.DataAvailable)
        {
            Byte[] inStream = new Byte[clientSocket.SendBufferSize];
            stream.Read(inStream, 0, inStream.Length);
            result += System.Text.Encoding.UTF8.GetString(inStream);
        }
        return result;
    }

    //disconnect from the socket
    public void closeSocket()
    {
        if (!isSocketReady)
            return;
        writer.Close();
        reader.Close();
        clientSocket.Close();
        isSocketReady = false;
    }

    //keep connection alive, reconnect if connection lost
    public void maintainConnection()
    {
        if (!stream.CanRead)
        {
            setupSocket();
        }
    }
}
