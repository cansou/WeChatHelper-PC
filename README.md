# WeChatHelper-PC

## 简介
1. 环境：VS2019、MySql5.7。
2. 只支持2.7.1.88版本。
3. 第一次接触C++，第一版代码写的惨不忍睹。
3. 只保存文字消息到Mysql，修改MySqlTool.app文件中配置，改成自己的MySql配置。
4. 表结构：
> CREATE TABLE `receive_msg` (
  `id` bigint(11) NOT NULL AUTO_INCREMENT COMMENT '主键',
  `from_wxid` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL COMMENT '消息来源，好友ID、或群ID',
  `sender_wxid` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL COMMENT '好友消息，为空。群消息，是发送者ID',
  `msg_type` int(255) DEFAULT NULL COMMENT '消息类型：1：文字、图片、语音、表情、好友确认、名片、视频、红包、撤回消息等',
  `content` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL COMMENT '消息内容',
  `receive_date` varchar(50) COLLATE utf8mb4_unicode_ci DEFAULT NULL COMMENT '接收消息时间',
  `from_type` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL COMMENT '消息来源。1：好友消息，2：群消息',
  `tenant` int(11) DEFAULT NULL COMMENT '租户字段，存放登录微信ID',
  PRIMARY KEY (`id`)
) 

## 相关bug
1. 保存到MySql中汉字是乱码
2. 不支持高并发

## RoadMap
1. 改bug，优化代码，减少硬编码
2. 发送消息。
3. 定时消息发送，自动回复等。
4. 微信多开
5. Socket，客户端控制dll
6. 其他...

## 参考资料
1. 网易云课堂《X86软件逆向分析实战》：https://study.163.com/course/courseMain.htm?courseId=1209042813
2. https://github.com/codeByDog/pcWechat
3. 还有很多提供帮助的朋友，不一一列举了。
感谢各位大佬！！！


