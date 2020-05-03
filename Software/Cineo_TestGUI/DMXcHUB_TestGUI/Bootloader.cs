using System;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System.Threading.Tasks;
using System.Threading;

namespace Bootloader
{
    public enum BootloaderCommand : byte
    {
        CMD_VERIFY_CHECKSUM = 0x31,
        CMD_GET_FLASH_SIZE = 0x32,
        CMD_GET_APP_STATUS = 0x33,
        CMD_ERASE_ROW = 0x34,
        CMD_SYNC = 0x35,
        CMD_SET_ACTIVE_APP = 0x36,
        CMD_SEND_DATA = 0x37,
        CMD_ENTER_BOOTLOADER = 0x38,
        CMD_PROGRAM_ROW = 0x39,
        CMD_VERIFY_ROW = 0x3A,
        CMD_EXIT_BOOTLOADER = 0x3B,
        CMD_GET_METADATA = 0x3C
    }

    public enum BootloaderStatus : ushort
    {
        CYRET_SUCCESS = 0x0000,
        BOOTLOADER_ERR_LENGTH = 0x0003,
        BOOTLOADER_ERR_DATA = 0x0004,
        BOOTLOADER_ERR_CMD = 0x0005,
        BOOTLOADER_ERR_CHECKSUM = 0x0008,
        BOOTLOADER_ERR_ARRAY = 0x0009,
        BOOTLOADER_ERR_ROW = 0x000A,
        BOOTLOADER_ERR_APP = 0x000C,
        BOOTLOADER_ERR_ACTIVE = 0x000D,
        BOOTLOADER_ERR_UNK = 0x000F
    }

    public enum HostStatus : ushort
    {
        CYRET_SUCCESS = 0x0000,
        CYRET_ERR_FILE = 0x0001,
        CYRET_ERR_EOF = 0x0002,
        CYRET_ERR_LENGTH = 0x0003,
        CYRET_ERR_DATA = 0x0004,
        CYRET_ERR_CMD = 0x0005,
        CYRET_ERR_DEVICE = 0x0006,
        CYRET_ERR_VERSION = 0x0007,
        CYRET_ERR_CHECKSUM = 0x0008,
        CYRET_ERR_ARRAY = 0x0009,
        CYRET_ERR_ROW = 0x000A,
        CYRET_ERR_BTLDR = 0x000B,
        CYRET_ERR_ACTIVE = 0x000C,
        CYRET_ERR_UNK = 0x000F,
        CYRET_ABORT = 0x00FF,
        CYRET_ERR_COMM_MASK = 0x2000,
        CYRET_ERR_BTLDR_MASK = 0x4000
    }

    public delegate int OpenConnection();
    public delegate int CloseConnection();
    public delegate int ReadData(Stream stream, int size);
    public delegate int WriteData(Stream stream, int size);
    public delegate void Update(ushort rowNum);

    public struct BootloaderSettings
    {
        public Stream CyacdStream;
        public OpenConnection OpenConnection;
        public CloseConnection CloseConnection;
        public ReadData ReadData;
        public WriteData WriteData;
        public Update Update;
        public int MaxTransferSize;
    }

    enum BootloaderAction
    {
        PROGRAM,
        ERASE,
        VERIFY,
        VERSION,
        TEST
    }

    enum BootloaderChecksum
    {
        SUM,
        CRC
    }

    public class Bootloader
    {
        private BootloaderSettings settings;
        private BootloaderChecksum checksumType;
        private uint[] validRows = new uint[4];
        private CancellationToken cancelToken;

        private const uint BL_VER_SUPPORT_VERIFY = 0x010214;

        private byte[] fromAscii(string hexData)
        {
            return Enumerable.Range(0, hexData.Length)
                .Where(x => x % 2 == 0)
                .Select(x => Convert.ToByte(hexData.Substring(x, 2), 16))
                .ToArray();
        }

        private int parseHeader(string header, ref uint siliconId, ref byte siliconRev, ref byte checksumType)
        {
            byte[] rowData = fromAscii(header);
            if (rowData.Length != 6)
                return (int)HostStatus.CYRET_ERR_LENGTH;

            siliconId = ((uint)rowData[0] << 24) | ((uint)rowData[1] << 16) | ((uint)rowData[2] << 8) | ((uint)rowData[3]);
            siliconRev = rowData[4];
            checksumType = rowData[5];

            return (int)HostStatus.CYRET_SUCCESS;
        }

        private int parseRowData(string lineData, ref byte arrayId, ref ushort rowNum, ref byte[] rowData, ref ushort size, ref byte checksum)
        {
            if (lineData == null)
                return (int)HostStatus.CYRET_ERR_EOF;

            byte[] hexData = fromAscii(new string(lineData.Skip(1).ToArray()));

            if (hexData.Length <= 6)
                return (int)HostStatus.CYRET_ERR_LENGTH;
            if (lineData.First() != ':')
                return (int)HostStatus.CYRET_ERR_CMD;

            arrayId = hexData[0];
            rowNum = (ushort)((hexData[1] << 8) | (hexData[2]));
            size = (ushort)((hexData[3] << 8) | (hexData[4]));
            checksum = (byte)((hexData[hexData.Length - 1]));

            if (size + 6 != hexData.Length)
                return (int)HostStatus.CYRET_ERR_DATA;

            rowData = hexData.Skip(5).Take(size).ToArray();

            return (int)HostStatus.CYRET_SUCCESS;
        }

        private ushort computeChecksum(byte[] packetData)
        {
            uint size = (uint)packetData.Length;
            if (checksumType == BootloaderChecksum.CRC)
            {
                uint index = 0;
                ushort crc = 0xFFFF;
                ushort tmp;

                if (size == 0)
                    return (ushort)~crc;

                do
                {
                    for (var i = 0; i < 8; i++)
                    {
                        tmp = (ushort)(0x00FF & packetData[index++]);

                        if (((crc & 0x0001) ^ (tmp & 0x0001)) != 0)
                            crc = (ushort)((crc >> 1) ^ 0x8408);
                        else
                            crc >>= 1;

                        tmp >>= 1;
                    }
                } while (--size > 0);

                crc = (ushort)~crc;
                tmp = crc;
                crc = (ushort)((crc << 8) | (tmp >> 8 & 0xFF));

                return crc;
            }
            else
            {
                uint index = 0;
                ushort sum = 0;
                while (size-- > 0)
                    sum += packetData[index++];

                return (ushort)(1 + ~sum);
            }
        }

        private byte[] transferData(byte[] packetData, int responseLength, bool hasResponse)
        {
            byte[] responseData = new byte[responseLength + 7];
            settings.WriteData(new MemoryStream(packetData), packetData.Length);

            if (hasResponse)
                settings.ReadData(new MemoryStream(responseData), responseLength + 7);

            return responseData;
        }

        private byte[] createPacket(BootloaderCommand packetCommand, byte[] packetData)
        {
            List<byte> packetOut = new List<byte>();

            packetOut.Add(0x01);
            packetOut.Add((byte)packetCommand);
            if (packetData != null)
                packetOut.AddRange(BitConverter.GetBytes((ushort)packetData.Length));
            else
                packetOut.AddRange(new byte[2] { 0x00, 0x00 });
            if (packetData != null)
                packetOut.AddRange(packetData);
            packetOut.AddRange(BitConverter.GetBytes(computeChecksum(packetOut.ToArray())));
            packetOut.Add(0x17);

            return packetOut.ToArray();
        }



        private int startBootload(uint expSiId, byte expSiRev)
        {
            int err = settings.OpenConnection();

            byte[] startRes;
            int retry = 10;
            do
            {
                startRes = transferData(createPacket(BootloaderCommand.CMD_ENTER_BOOTLOADER, null), 8, true);
            } while ((BootloaderStatus)startRes[1] != BootloaderStatus.CYRET_SUCCESS && --retry != 0);

            if ((BootloaderStatus)startRes[1] != BootloaderStatus.CYRET_SUCCESS)
                return (int)startRes[1] | 0x4000;

            uint siliconId = (uint)(((startRes[7] << 24) | (startRes[6] << 16) | (startRes[5] << 8) | startRes[4]));
            byte siliconRev = startRes[8];
            uint blVersion = (uint)((startRes[11] << 16) | (startRes[10] << 8) | startRes[9]);

            if (expSiId != siliconId || expSiRev != siliconRev)
                return (int)HostStatus.CYRET_ERR_DEVICE;

            return (int)HostStatus.CYRET_SUCCESS;
        }

        private int validateRow(byte arrayId, ushort rowNum)
        {
            if (arrayId >= 4)
                return (int)HostStatus.CYRET_ERR_ARRAY;

            byte[] flashSizeRes = transferData(createPacket(BootloaderCommand.CMD_GET_FLASH_SIZE, new byte[] { arrayId }), 4, true);
            if ((BootloaderStatus)flashSizeRes[1] != BootloaderStatus.CYRET_SUCCESS)
                return (int)flashSizeRes[1] | 0x4000;

            ushort startRow = (ushort)((flashSizeRes[5] << 8) | flashSizeRes[4]);
            ushort endRow = (ushort)((flashSizeRes[7] << 8) | flashSizeRes[6]);

            validRows[arrayId] = ((uint)startRow << 16) + endRow;

            startRow = (ushort)(validRows[arrayId] >> 16);
            endRow = (ushort)validRows[arrayId];

            if (rowNum < startRow || rowNum > endRow)
                return (int)HostStatus.CYRET_ERR_ROW;

            return (int)HostStatus.CYRET_SUCCESS;
        }

        private int eraseRow(byte arrayId, ushort rowNum)
        {
            int err = validateRow(arrayId, rowNum);
            if (err != (int)HostStatus.CYRET_SUCCESS)
                return err;

            List<byte> dataBytes = new List<byte>();
            dataBytes.Add(arrayId);
            dataBytes.AddRange(BitConverter.GetBytes(rowNum));

            byte[] eraseRes = transferData(createPacket(BootloaderCommand.CMD_ERASE_ROW, dataBytes.ToArray()), 0, true);
            if ((BootloaderStatus)eraseRes[1] != BootloaderStatus.CYRET_SUCCESS)
                return (int)eraseRes[1] | 0x4000;

            return (int)HostStatus.CYRET_SUCCESS;
        }

        private int programRow(byte arrayId, ushort rowNum, byte[] buffer)
        {
            ushort size = (ushort)buffer.Length;

            int err = validateRow(arrayId, rowNum);
            if (err != (int)HostStatus.CYRET_SUCCESS)
                return err;

            int offset = 0;
            while (err == (int)HostStatus.CYRET_SUCCESS && (size - offset + 11) > settings.MaxTransferSize)
            {
                int sendDataSize = settings.MaxTransferSize - 11;

                byte[] sendDataRes = transferData(createPacket(BootloaderCommand.CMD_SEND_DATA, buffer.Skip(offset).Take(sendDataSize).ToArray()), 0, true);
                if ((BootloaderStatus)sendDataRes[1] != BootloaderStatus.CYRET_SUCCESS)
                    return (int)sendDataRes[1] | 0x4000;

                offset += sendDataSize;
            }

            if (err != (int)HostStatus.CYRET_SUCCESS)
                return err;

            int programRowSize = size - offset;

            List<byte> dataBytes = new List<byte>();
            dataBytes.Add(arrayId);
            dataBytes.AddRange(BitConverter.GetBytes(rowNum));
            dataBytes.AddRange(buffer.Skip(offset).Take(programRowSize));

            byte[] programRowRes = transferData(createPacket(BootloaderCommand.CMD_PROGRAM_ROW, dataBytes.ToArray()), 0, true);
            if ((BootloaderStatus)programRowRes[1] != BootloaderStatus.CYRET_SUCCESS)
                return (int)programRowRes[1] | 0x4000;

            return (int)HostStatus.CYRET_SUCCESS;
        }

        private int verifyRow(byte arrayId, ushort rowNum, byte checksum)
        {
            int err = validateRow(arrayId, rowNum);
            if (err != (int)HostStatus.CYRET_SUCCESS)
                return err;

            List<byte> dataBytes = new List<byte>();
            dataBytes.Add(arrayId);
            dataBytes.AddRange(BitConverter.GetBytes(rowNum));

            byte[] validateRowRes = transferData(createPacket(BootloaderCommand.CMD_VERIFY_ROW, dataBytes.ToArray()), 1, true);
            if ((BootloaderStatus)validateRowRes[1] != BootloaderStatus.CYRET_SUCCESS)
                return (int)validateRowRes[1] | 0x4000;

            byte rowChecksum = validateRowRes[4];
            if (rowChecksum != checksum)
                return (int)HostStatus.CYRET_ERR_CHECKSUM;

            return (int)HostStatus.CYRET_SUCCESS;
        }

        private int verifyApplication()
        {
            byte[] verifyApplicationRes = transferData(createPacket(BootloaderCommand.CMD_VERIFY_CHECKSUM, null), 1, true);
            if ((BootloaderStatus)verifyApplicationRes[1] != BootloaderStatus.CYRET_SUCCESS)
                return (int)verifyApplicationRes[1] | 0x4000;

            byte checksumValid = verifyApplicationRes[4];

            if (checksumValid == 0)
                return (int)HostStatus.CYRET_ERR_CHECKSUM;

            return (int)HostStatus.CYRET_SUCCESS;
        }

        private int runAction(BootloaderAction action, Stream fileStream)
        {
            uint siliconId = 0;
            byte siliconRev = 0;
            byte checksumType = 0;
            int err = (int)HostStatus.CYRET_SUCCESS;

            MemoryStream ms = new MemoryStream();
            fileStream.CopyTo(ms);

            int currentRow = 0;
            int rowCount = getRows(ms)-2;

            using (StreamReader cyacdReader = new StreamReader(ms))
            {
                err = parseHeader(cyacdReader.ReadLine(), ref siliconId, ref siliconRev, ref checksumType);

                this.checksumType = (checksumType == 1) ? BootloaderChecksum.CRC : BootloaderChecksum.SUM;

                if (err == (int)HostStatus.CYRET_SUCCESS)
                    err = startBootload(siliconId, siliconRev);

                if (action == BootloaderAction.VERSION)
                {
                    if (err == (int)HostStatus.CYRET_SUCCESS)
                    {
                        byte[] getMetadataRes = transferData(createPacket(BootloaderCommand.CMD_GET_METADATA, new byte[1] { 0x00 }), 0x56, true);
                        if ((BootloaderStatus)getMetadataRes[1] != BootloaderStatus.CYRET_SUCCESS)
                            err = getMetadataRes[1] | 0x4000;
                        else
                        {
                            err = ((ushort)getMetadataRes[0x19] << 8) | getMetadataRes[0x1A];
                            transferData(createPacket(BootloaderCommand.CMD_EXIT_BOOTLOADER, new byte[1] { 0 }), 0, false);
                            return err;
                        }
                    }
                }
                else
                {
                    ushort startRow = 0;

                    while (err == (int)HostStatus.CYRET_SUCCESS)
                    {
                        byte arrayId = 0;
                        ushort rowNum = 0;
                        ushort size = 0;
                        byte checksum = 0;
                        byte[] rowData = null;

                        err = parseRowData(cyacdReader.ReadLine(), ref arrayId, ref rowNum, ref rowData, ref size, ref checksum);

                        if (startRow == 0)
                            startRow = rowNum;

                        if (err != (int)HostStatus.CYRET_SUCCESS)
                            break;

                        switch (action)
                        {
                            case BootloaderAction.TEST:
                                err = (int)HostStatus.CYRET_ERR_EOF;
                                break;
                            case BootloaderAction.ERASE:
                                err = eraseRow(arrayId, rowNum);
                                break;
                            case BootloaderAction.PROGRAM:
                                int retry = 5;
                                do
                                {
                                    err = programRow(arrayId, rowNum, rowData);
                                } while (--retry != 0 && err != 0);
                                if (err == (int)HostStatus.CYRET_SUCCESS)
                                    goto case BootloaderAction.VERIFY;
                                break;
                            case BootloaderAction.VERIFY:
                                byte checksum2 = (byte)(checksum + arrayId + (ushort)rowNum + (ushort)(rowNum >> 8) + (ushort)rowData.Length + (ushort)(rowData.Length >> 8));
                                err = verifyRow(arrayId, rowNum, checksum2);
                                break;
                        }

                        if (cancelToken != null && cancelToken.IsCancellationRequested)
                        {
                            err = (int)HostStatus.CYRET_ABORT;
                        }
                        settings.Update((ushort)((float)currentRow++ / rowCount * 100.0f));
                    }
                }

                transferData(createPacket(BootloaderCommand.CMD_EXIT_BOOTLOADER, new byte[1] { 0 }), 0, false);
                settings.CloseConnection();
            }

            if (err == (int)HostStatus.CYRET_ERR_EOF)
                return (int)HostStatus.CYRET_SUCCESS;
            return err;
        }

        private int getRows(Stream stream)
        {
            if (stream.CanSeek)
            {
                stream.Seek(0, SeekOrigin.Begin);
                StreamReader reader = new StreamReader(stream);
                int i = 0;
                while (reader.ReadLine() != null) { i++; }
                stream.Seek(0, SeekOrigin.Begin);
                return i - 1;
            }

            return 0;
        }

        public int Program(Stream fileStream, CancellationToken cancelToken)
        {
            this.cancelToken = cancelToken;
            return runAction(BootloaderAction.PROGRAM, fileStream);
        }

        public int Erase(Stream fileStream)
        {
            return runAction(BootloaderAction.ERASE, fileStream);
        }

        public int Verify(Stream fileStream)
        {
            return runAction(BootloaderAction.VERIFY, fileStream);
        }

        public int Version(Stream fileStream)
        {
            return runAction(BootloaderAction.VERSION, fileStream);
        }

        public int Test(Stream fileStream)
        {
            return runAction(BootloaderAction.TEST, fileStream);
        }

        public Bootloader(BootloaderSettings settings)
        {
            this.settings = settings;
        }
    }
}
