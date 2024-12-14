# -*- coding: utf-8 -*-
import logging

global root_logger
def log_init(log_file_path=None, use_as_root=True):    
    # 创建一个Logger对象
    local_logger = logging.getLogger()
    local_logger.setLevel(logging.INFO)

    # 创建一个日志格式化器
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')

    if log_file_path is not None:
        # 创建一个文件处理器
        file_handler = logging.FileHandler(log_file_path)
        # 将文件处理器添加到Logger中
        local_logger.addHandler(file_handler)
        file_handler.setFormatter(formatter)
    else:
        # 创建一个控制台处理器
        console_handler = logging.StreamHandler()
        # 将控制台处理器添加到Logger中
        local_logger.addHandler(console_handler)
        console_handler.setFormatter(formatter)

    # 输出日志信息
    local_logger.info('创建日志成功。')

    if use_as_root:
        global root_logger
        root_logger = local_logger

    return local_logger
