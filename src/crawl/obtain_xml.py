import requests
import xml.etree.ElementTree as ET
import os
import time  # 导入时间模块


def download_xml(url, save_path=None, headers=None, delay=0):
    """
    delay: 每次抓取前等待的秒数，默认0
    """
    # 1. 强制延迟，遵守 robots.txt 协议
    if delay > 0:
        print(f"遵照协议，等待 {delay} 秒...")
        time.sleep(delay)

    if headers is None:
        headers = {
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36',
        }

    try:
        # 2. 使用 stream=True 可以处理大型文件
        response = requests.get(url, headers=headers, timeout=30, stream=True)
        response.raise_for_status()

        # 3. 如果文件很大，建议分块读取，这里简单处理
        xml_content = response.content

        # 验证有效性 (ElementTree 在处理带命名空间的 XML 时可能需要额外处理)
        try:
            ET.fromstring(xml_content)
        except ET.ParseError:
            print("警告: 抓取的内容不是标准 XML 格式")

        if save_path:
            # 确保目录存在
            folder = os.path.dirname(save_path)
            if folder:  # 预防 save_path 只是个文件名的情况
                os.makedirs(folder, exist_ok=True)
            with open(save_path, 'wb') as f:
                f.write(xml_content)

        return xml_content

    except requests.RequestException as e:
        print(f"下载失败: {e}")
        return None


# 使用示例
# if __name__ == "__main__":
#     # 针对你刚才看的路透社站点地图，设置 delay=10
#     xml_url = "https://www.people.com.cn/rss/politics.xml"
#     content = download_xml(xml_url, "./people_yuliao2/politics.xml",
#                             delay=10)
