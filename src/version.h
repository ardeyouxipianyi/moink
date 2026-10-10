#ifndef MOINK_VERSION_H
#define MOINK_VERSION_H

/*
 * 版本号（R1.2.0 起统一为一套）：
 *
 *   MOINK_VERSION      唯一版本号 —— 固件与内嵌页面共用这一个。
 *   MOINK_API_VERSION  帧格式契约号 —— 与发布版本号解耦：16 字节头 + 2bpp 载荷，
 *                      载荷几何 768x552（hdr[2] = 1）。hdr[2] = 2（A1 原生 800x600
 *                      对照档）已随 R1.2.2 删除。
 *                      它**不对外显示为「版本」**，页面状态栏另有一行「接口」。
 *                      仅当帧头字段/偏移或载荷契约变化时才 +1；api 2 自 R1.1.0 起未变
 *                      （R1.2.2 只收窄了接受的几何集合，帧头格式零变动，故不加号）。
 *
 * 编号规则（沿用本项目既有节奏，只是不再分 fw / page 两条线）：
 *   主版本 +1 —— 破坏性变更（分区表 / NVS 布局 / 屏驱动基线 / 帧格式契约）
 *   次版本 +1 —— 固件功能批（新接口、新路由、新能力）
 *   第三位 +1 —— 修复批、纯页面变更（含页面热更）
 *
 * 运行时「当前版本」= 当前生效页面的版本：设备上存在页面热更时取热更页
 * <meta name="moink-page-version"> 的值，否则等于 MOINK_VERSION。页面可单独
 * 热更，故它恒 >= 固件编译进去的号（见 ota_web_page_version()）。
 *
 * R1.2.0 变更（2026-09-24，FB-015）：
 *   - 版本号由 fw / page 两套合并为 MOINK_VERSION 一套；删除 MOINK_FW_VERSION /
 *     MOINK_PAGE_VERSION，/api/info 的 fw / page 两个字段改为单一 ver。
 *   - 升级入口合并：POST /api/upload 按文件首块内容自动识别固件（.bin，首字节
 *     0xE9）或控制页（.html，含 <!doctype / <html），分别走 OTA 与 web 分区热更。
 *     原 /api/ota、/api/web 路由与 ?sync_page 查询参数整体删除。
 *   - 升级行为：上传固件时页面一并升级（OTA 校验通过后清除页面热更标记，不再需要
 *     ?sync_page=1）；上传控制页时只换页面，不重启。
 *   - 每次固件升级强制重置设备设置：擦除除热点名/密码以外的所有设置键，避免旧状态
 *     带病升级（见 settings_reset_for_upgrade()）。**该行为已在 R1.2.2 撤销** ——
 *     现改为只清理已废弃的键，用户设置一律保留，详见下方 R1.2.2 变更。
 *   - 不保留向后兼容：页面与固件必须同批交付。api 保持 2（帧格式零变动）。
 *   - 清除过时代码：A1.1 诊断变体（V1~V8）与 EPD_PANEL_A11 画像、MOINK_EMBED_PAGE
 *     条件编译与占位页、a1_mode 旧值兼容归一、ota_web_capacity() 死代码。
 *
 * 升级判据：改的是页面还是固件由「统一入口上传的文件类型」决定，不再需要人判断；
 * 版本号推进幅度按上表取值。
 *
 * R1.2.1 变更（2026-10-04）：
 *   - 修复 captive portal 探测请求未进入处理函数、直接返回 404 的问题，并完善
 *     DHCP DNS 与 A、AAAA、HTTPS 等 DNS 查询响应；不向 iPhone 下发需要可信 HTTPS
 *     API 的 Option 114，改由 DNS 劫持和 HTTP 探测重定向触发门户。兼容 Apple DNS
 *     多问题查询，并按 Espressif 官方示例扩充 captive 探测并发 socket；Apple 探测
 *     地址返回轻量 Safari 引导页，复制地址后显示友好完成页，后台复探返回标准 Success，
 *     热点保持连接供 Safari 打开正式配置页；客户端断开后立即清除 Success 状态，下次
 *     连接重新显示引导页，60 秒超时仅作兜底。
 *   - WiFi 发射功率默认档由高（18dBm）调整为中（10dBm），解决部分ESP32 C3 supermini在默认高功率手机很难连接问题。
 *   - 缩放、裁剪图片时松手再做计算，体验更丝滑
 *   - 缩放、裁剪图片时松手再做计算，体验更丝滑
 *   - 文本框在localStorage保存导致下次打开时文本仍在问题解决
 *
 * R1.2.2 变更（2026-10-10）：
 *   - 屏幕设置去掉「A1 驱动」二级菜单：A1 真机验收通过后，只保留 768x552 顺序
 *     直写一种策略；800x600 对照诊断档连同其固件接收路径一并删除。api 仍为 2
 *     ——帧头格式零变动，只是不再接受该对照几何。
 *   - 新增「GPIO 设置」：SPI 六线（SCK/MOSI/CS/DC/RST/BUSY）与电池 ADC、唤醒键
 *     改为运行时可配，提供「默认 / 排线 / 整合板 / 自定义」四档；自定义由页面逐个
 *     选 IO，固件侧做白名单 + 去重校验（仅 GPIO0..10，且避开自锁脚 2）。引脚在
 *     启动时由 epd_init()/power_init() 读取，改后需重启生效（新增 POST /api/reboot）。
 *   - 新增 MOS 一键开机自锁脚 GPIO2：上电第一件事拉高，深睡前 gpio_hold 锁存以维持
 *     外部自锁电路持续供电。
 *   - **撤销 R1.2.0 的「每次固件升级强制重置设置」**：改为只清理本版已废弃的键
 *     （a1_mode / a11_var），屏型 / 翻转 / 功率 / GPIO 接法 / 休眠 / 唤醒 / 热点
 *     凭据全部原样保留。全量重置会误伤描述硬件的项 —— A1 设备回落 A0 驱动、排线 /
 *     整合板接法回落默认引脚，升级完得重新配一遍甚至先看到「屏不亮」。
 *   - /api/info、/api/settings 去掉 a1_mode，新增 gpio_preset（settings 另带 pins[8]）。
 */
#define MOINK_API_VERSION   2
#define MOINK_VERSION       "R1.2.2"

#endif /* MOINK_VERSION_H */
