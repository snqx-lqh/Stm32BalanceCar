import 'package:flutter/material.dart';
import 'dart:io';
import 'dart:async';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Balance Car App',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        fontFamily: 'Pacifico', // 设置全局艺术字体
      ),
      home: BalanceCarScreen(),
    );
  }
}

class BalanceCarScreen extends StatefulWidget {
  @override
  _BalanceCarScreenState createState() => _BalanceCarScreenState();
}

class _BalanceCarScreenState extends State<BalanceCarScreen> {
  // 用于存储用户输入的 IP/PORT
  final TextEditingController _ipController = TextEditingController();
  final TextEditingController _portController = TextEditingController();
  // TCP 连接状态
  bool _isConnected = false;
  // TCP Socket 对象
  Socket? _socket;
  // 长按定时器
  Timer? _longPressTimer;

  // 连接或断开连接
  Future<void> _connectOrDisconnect() async {
    if (_isConnected) {
      // 断开连接
      _socket?.destroy();
      setState(() {
        _isConnected = false;
        _socket = null;
      });
    } else {
      // 获取 IP 和端口
      String ip = _ipController.text;
      int port = int.tryParse(_portController.text) ?? 0;

      if (ip.isEmpty || port == 0) {
        _showErrorDialog('请输入正确的 IP 和端口');
        return;
      }

      try {
        // 尝试连接
        _socket = await Socket.connect(ip, port, timeout: Duration(seconds: 5));
        setState(() {
          _isConnected = true;
        });
      } catch (e) {
        _showErrorDialog('连接失败：$e');
      }
    }
  }

  // 显示错误对话框
  void _showErrorDialog(String message) {
    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: Text('错误'),
          content: Text(message),
          actions: [
            TextButton(
              onPressed: () {
                Navigator.of(context).pop();
              },
              child: Text('确定'),
            ),
          ],
        );
      },
    );
  }

  // 发送字符
  void _sendCommand(String command) {
    if (_isConnected && _socket != null) {
      _socket!.write(command);
    }
  }

  // 长按开始发送
  void _startLongPress(String command) {
    _longPressTimer = Timer.periodic(Duration(milliseconds: 200), (timer) {
      _sendCommand(command);
    });
  }

  // 长按结束
  void _endLongPress() {
    _longPressTimer?.cancel();
    _longPressTimer = null;
  }

  @override
  void dispose() {
    _socket?.destroy();
    _longPressTimer?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Container(
        // 整体背景设置为蓝色渐变
        decoration: BoxDecoration(
          gradient: LinearGradient(
            begin: Alignment.topCenter,
            end: Alignment.bottomCenter,
            colors: [
              Colors.blue.shade700,
              Colors.blue.shade500,
              Colors.blue.shade300,
            ],
          ),
        ),
        child: Column(
          children: [
            // 上半部分：平衡小车显示区域
            Expanded(
              flex: 1,
              child: Center(
                child: Text(
                  '平衡小车',
                  style: TextStyle(
                    fontSize: 48,
                    fontWeight: FontWeight.bold,
                    color: Colors.white,
                  ),
                ),
              ),
            ),
            // 下半部分：按钮区域
            Expanded(
              flex: 2,
              child: Padding(
                padding: const EdgeInsets.all(20.0),
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    // IP/PORT 输入行
                    Container(
                      padding: EdgeInsets.symmetric(horizontal: 20, vertical: 10),
                      decoration: BoxDecoration(
                        color: Colors.white.withOpacity(0.2),
                        borderRadius: BorderRadius.circular(10),
                      ),
                      child: Row(
                        crossAxisAlignment: CrossAxisAlignment.center,
                        children: [
                          Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              // IP 输入框
                              Row(
                                children: [
                                  Text(
                                    'IP:',
                                    style: TextStyle(
                                      fontSize: 16,
                                      color: Colors.white,
                                    ),
                                  ),
                                  SizedBox(width: 10),
                                  SizedBox(
                                    width: 120,
                                    child: TextField(
                                      controller: _ipController,
                                      style: TextStyle(color: Colors.white),
                                      decoration: InputDecoration(
                                        border: InputBorder.none,
                                        hintText: '请输入IP',
                                        hintStyle: TextStyle(color: Colors.white70),
                                      ),
                                    ),
                                  ),
                                ],
                              ),
                              SizedBox(height: 10),
                              // PORT 输入框
                              Row(
                                children: [
                                  Text(
                                    'PORT:',
                                    style: TextStyle(
                                      fontSize: 16,
                                      color: Colors.white,
                                    ),
                                  ),
                                  SizedBox(width: 10),
                                  SizedBox(
                                    width: 120,
                                    child: TextField(
                                      controller: _portController,
                                      style: TextStyle(color: Colors.white),
                                      keyboardType: TextInputType.number,
                                      decoration: InputDecoration(
                                        border: InputBorder.none,
                                        hintText: '请输入端口',
                                        hintStyle: TextStyle(color: Colors.white70),
                                      ),
                                    ),
                                  ),
                                ],
                              ),
                            ],
                          ),
                          SizedBox(width: 20),
                          // 连接按钮
                          ElevatedButton(
                            onPressed: _connectOrDisconnect,
                            style: ElevatedButton.styleFrom(
                              backgroundColor: _isConnected ? Colors.green : Colors.blue.shade800,
                              foregroundColor: Colors.white,
                            ),
                            child: Text(_isConnected ? '断开' : '连接'),
                          ),
                        ],
                      ),
                    ),
                    SizedBox(height: 40),
                    // 上按钮
                    GestureDetector(
                      onTap: () => _sendCommand('F'),
                      onLongPressStart: (_) => _startLongPress('F'),
                      onLongPressEnd: (_) => _endLongPress(),
                      child: ElevatedButton(
                        onPressed: null,
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.white.withOpacity(0.2),
                          foregroundColor: Colors.white,
                          minimumSize: Size(80, 80),
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(10),
                          ),
                        ),
                        child: Text(
                          '↑',
                          style: TextStyle(fontSize: 32),
                        ),
                      ),
                    ),
                    SizedBox(height: 20),
                    // 左、中、右按钮
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                      children: [
                        // 左按钮
                        GestureDetector(
                          onTap: () => _sendCommand('L'),
                          onLongPressStart: (_) => _startLongPress('L'),
                          onLongPressEnd: (_) => _endLongPress(),
                          child: ElevatedButton(
                            onPressed: null,
                            style: ElevatedButton.styleFrom(
                              backgroundColor: Colors.white.withOpacity(0.2),
                              foregroundColor: Colors.white,
                              minimumSize: Size(80, 80),
                              shape: RoundedRectangleBorder(
                                borderRadius: BorderRadius.circular(10),
                              ),
                            ),
                            child: Text(
                              '←',
                              style: TextStyle(fontSize: 32),
                            ),
                          ),
                        ),
                        // 中间按钮
                        GestureDetector(
                          onTap: () => _sendCommand('S'),
                          onLongPressStart: (_) => _startLongPress('S'),
                          onLongPressEnd: (_) => _endLongPress(),
                          child: ElevatedButton(
                            onPressed: null,
                            style: ElevatedButton.styleFrom(
                              backgroundColor: Colors.white.withOpacity(0.2),
                              foregroundColor: Colors.white,
                              minimumSize: Size(80, 80),
                              shape: CircleBorder(),
                            ),
                            child: Text(
                              '⚪',
                              style: TextStyle(fontSize: 32),
                            ),
                          ),
                        ),
                        // 右按钮
                        GestureDetector(
                          onTap: () => _sendCommand('R'),
                          onLongPressStart: (_) => _startLongPress('R'),
                          onLongPressEnd: (_) => _endLongPress(),
                          child: ElevatedButton(
                            onPressed: null,
                            style: ElevatedButton.styleFrom(
                              backgroundColor: Colors.white.withOpacity(0.2),
                              foregroundColor: Colors.white,
                              minimumSize: Size(80, 80),
                              shape: RoundedRectangleBorder(
                                borderRadius: BorderRadius.circular(10),
                              ),
                            ),
                            child: Text(
                              '→',
                              style: TextStyle(fontSize: 32),
                            ),
                          ),
                        ),
                      ],
                    ),
                    SizedBox(height: 20),
                    // 下按钮
                    GestureDetector(
                      onTap: () => _sendCommand('B'),
                      onLongPressStart: (_) => _startLongPress('B'),
                      onLongPressEnd: (_) => _endLongPress(),
                      child: ElevatedButton(
                        onPressed: null,
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.white.withOpacity(0.2),
                          foregroundColor: Colors.white,
                          minimumSize: Size(80, 80),
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(10),
                          ),
                        ),
                        child: Text(
                          '↓',
                          style: TextStyle(fontSize: 32),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}