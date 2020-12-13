
TRANTOR_SOURCES_PATH=./dependencies/trantor

g++  -I ./dependencies/trantor \
       -I ./dependencies/trantor/include \
       -I $TRANTOR_SOURCES_PATH/trantor/utils \
       -I $TRANTOR_SOURCES_PATH/trantor/net \
       -I $TRANTOR_SOURCES_PATH/trantor/net/inner \
       -L ~/Programs/clang/lib -lpthread   -o draft-http-tunnel -O3 ./src/main.cpp \
       $TRANTOR_SOURCES_PATH/trantor/utils/AsyncFileLogger.cc \
    $TRANTOR_SOURCES_PATH/trantor/utils/ConcurrentTaskQueue.cc \
    $TRANTOR_SOURCES_PATH/trantor/utils/Date.cc \
    $TRANTOR_SOURCES_PATH/trantor/utils/LogStream.cc \
    $TRANTOR_SOURCES_PATH/trantor/utils/Logger.cc \
    $TRANTOR_SOURCES_PATH/trantor/utils/MsgBuffer.cc \
    $TRANTOR_SOURCES_PATH/trantor/utils/SerialTaskQueue.cc \
    $TRANTOR_SOURCES_PATH/trantor/utils/TimingWheel.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/EventLoop.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/EventLoopThread.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/EventLoopThreadPool.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/InetAddress.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/TcpClient.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/TcpServer.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/Channel.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/inner/Acceptor.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/inner/Connector.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/inner/Poller.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/inner/Socket.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/inner/TcpConnectionImpl.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/inner/Timer.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/inner/TimerQueue.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/inner/poller/EpollPoller.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/inner/NormalResolver.cc \
    $TRANTOR_SOURCES_PATH/trantor/net/inner/poller/KQueue.cc   --std=c++2a
