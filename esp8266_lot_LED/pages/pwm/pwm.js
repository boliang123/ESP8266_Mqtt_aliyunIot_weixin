
import mqtt from '../../utils/mqtt.js';

const aliyunOpt = require('../../utils/aliyun/aliyun_connect.js');
let that = null;

var app = getApp();

Page({
  data: {
    light: "",
    currentTemperature:"",
    currentHumidity:"",
    currentTemperature1:"",
    currentHumidity1:"",
    currentyali:"",
    a0: "A0读取值",
    wendu:"温度1",
    shidu:"湿度1",
    wendu1:"温度2",
    shidu1:"湿度2",
    yali:"压力",
    pwm:"PWM",
    backgroundimage: false,
    buzzerOn: true,
    servo: 0 ,
    cradle: 0,

    

    client: null,//记录重连的次数
    reconnectCounts: 0,//MQTT连接的配置
    options: {
      protocolVersion: 4, //MQTT连接协议版本
      clean: false,
      reconnectPeriod: 1000, //1000毫秒，两次重新连接之间的间隔
      connectTimeout: 30 * 1000, //1000毫秒，两次重新连接之间的间隔
      resubscribe: true, //如果连接断开并重新连接，则会再次自动订阅已订阅的主题（默认true）
      clientId: 'a1Cez6S56MY.wechat|securemode=2,signmethod=hmacsha256,timestamp=1684524867232|',
      password: '074b44f7e1814d7bd6396920bc10904ce9648660fa20df75f4005b0ddcbb5938',
      username: 'wechat&a1Cez6S56MY',
     
    },

    aliyunInfo: {
      productKey: wx.getStorageSync("productKey"), //阿里云连接的三元组 ，请自己替代为自己的产品信息!!
      deviceName: wx.getStorageSync("deviceName"), //阿里云连接的三元组 ，请自己替代为自己的产品信息!!
      deviceSecret: wx.getStorageSync("deviceSecret"), //阿里云连接的三元组 ，请自己替代为自己的产品信息!!
      regionId: 'cn-shanghai', //阿里云连接的三元组 ，请自己替代为自己的产品信息!!
      pubTopic: wx.getStorageSync("pubTopic"), //发布消息的主题
      pubTopic2: '/a1ITu4LFqZf/TEMP2/user/weixinDuan', //自定义的topic（发布消息的主题）
      subTopic: '/sys/a1ITu4LFqZf/TEMP1/thing/service/property/set', //订阅消息的主题
    },
  },

  onLoad: function (){
    this.connect(); 
  },

  connect() {
    let client = aliyunOpt.getAliyunIotMqttClient('wxs://iot-as-mqtt.cn-shanghai.aliyuncs.com/mqtt', {
      clientId: 'a1Cez6S56MY.wechat|securemode=2,signmethod=hmacsha256,timestamp=1684432949645|',
      username: 'wechat&a1Cez6S56MY',
      password:'af9c9ba6797dfafc82e3fd7ab7f0bfdb5b34c15862b7dda4eaa034b1e32997b9',
    })

    client.on('connect', () => {
      console.log('Connected to MQTT server.')
      client.subscribe('/sys/a1Cez6S56MY/wechat/thing/event/property/post', (err) => {
        if (!err) {
          console.log('Subscribed to MQTT topic.')
        }
      })
      this.setData({
        client: client,
      })
    })

    client.on('message', (topic, message) => {
      let msg = JSON.parse(message.toString())
      console.log(`Received message: ${message}`)
      this.setData({
        temperature: msg.data.temperature,
        humidity: msg.data.humidity,
      })
    })

    client.on('error', (err) => {
      console.log('MQTT connection error', err)
    })
  },

  onClickOpen() {
    that.sendCommond('set',1);
  },
  onClickOff() {
    that.sendCommond('set', 0);
  },

  sendCommond(cmd, data) {
    let sendData = {
      cmd: cmd,
      data: data,
    };
    if (this.data.client && this.data.client.connected) {
      this.data.client.publish(this.data.aliyunInfo.pubTopic2, JSON.stringify(sendData));
      console.log(`发送的数据：${JSON.stringify(sendData)}`);
    } else {
      wx.showToast({
        title: '请先连接服务器',
        icon: 'none',
        duration: 2000
      })
    }
 
},
 




  onPullDownRefresh: function () {
    this.connect();
    this.setData({
      
    a0: wx.getStorageSync("a0"),
    wendu: wx.getStorageSync("wendu"),
    yali: wx.getStorageSync('yali'),
     pwm: wx.getStorageSync("pwm"),
    })
    wx.stopPullDownRefresh()
  },
  a0: function (e){
    let value = e.detail.value
    wx.setStorageSync("a0", value)
  },
  wendu: function (e) {
    let value = e.detail.value
    wx.setStorageSync("wendu", value)
  },
  yali : function (e) {
    let value = e.detail.value
    wx.setStorageSync("yali", value)
  },
  pwm: function (e){
    let value = e.detail.value
    wx.setStorageSync("pwm", value)
  },
   servo:function(e) {
    let value = e.detail.value
    wx.setStorageSync("servo", value)
  },

  connect: function () {
    that = this;
    let clientOpt = aliyunOpt.getAliyunIotMqttClient({
      productKey: that.data.aliyunInfo.productKey,
      deviceName: that.data.aliyunInfo.deviceName,
      deviceSecret: that.data.aliyunInfo.deviceSecret,
      regionId: that.data.aliyunInfo.regionId,
      port: that.data.aliyunInfo.port,
    });
    console.log(clientOpt);
    console.log("get data:" + JSON.stringify(clientOpt));
    let host = 'wxs://' + clientOpt.host;
    console.log("get data:" + JSON.stringify(clientOpt));

    that.data.options.clientId = clientOpt.clientId,
      that.data.options.password = clientOpt.password,
      that.data.options.username = clientOpt.username,

      console.log("这就是网址" + host);
    console.log("this.data.options data:" + JSON.stringify(this.data.options));

    this.data.client = mqtt.connect(host, this.data.options);
    console.log(this.data.client);

    this.data.client.on(
      'connect',
      function (connack) {
        wx.showToast({ title: '连接成功' })
        that.sendCommond("");
      }
    )

    that.data.client.on("message", function (topic, payload) {
      console.log(" 收到 topic:" + topic + " , payload :" + payload)
    
      let payloadObj = JSON.parse(payload);
      let currentTemperatureValue = payloadObj?.items?.CurrentTemperature?.value;
      let currentHumidityValue = payloadObj?.items?.CurrentHumidity?.value;
      let currentTemperatureValue1 = payloadObj?.items?.CurrentTemperature1?.value;
      let currentHumidityValue1 = payloadObj?.items?.CurrentHumidity1?.value;
      let currentyaliValue = payloadObj?.items?.Pressure?.value;
      if ((currentTemperatureValue !== undefined || currentTemperatureValue !== null)&&(currentHumidityValue !== undefined || currentHumidityValue !== null)&&(currentTemperatureValue1 !== undefined || currentTemperatureValue1 !== null)&&(currentHumidityValue1 !== undefined || currentHumidityValue1 !== null)) {
        that.setData({
          light: payloadObj.light,
          currentTemperature: currentTemperatureValue,
          currentHumidity:currentHumidityValue,
          currentTemperature1: currentTemperatureValue1,
          currentHumidity1:currentHumidityValue1,
          // currentyali:currentyaliValue
        })
        console.log("当前设备1温度：" + currentTemperatureValue);
        console.log("当前设备1湿度：" + currentHumidityValue);
        console.log("当前设备2温度：" + currentTemperatureValue1);
        console.log("当前设备2湿度：" + currentHumidityValue1);
        // console.log("当前压力：" + currentyaliValue);
      } else {
        console.log("payload中的items.CurrentTemperature未定义。");
      }
    })
  },
 

})