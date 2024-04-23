#!/usr/bin/python
# -*- coding: UTF-8 -*-
# from win10toast import ToastNotifier
from tkinter import *
import threading
import time


def say_hi():
    print("hello ~ !")


def my_timer_function():
    # print(f"定时器触发了！")
    while 1:
        root = Tk()

        frame1 = Frame(root)
        frame2 = Frame(root)
        root.title("喝水记时")

        label = Label(frame1, text="Label", justify=LEFT)
        label.pack(side=LEFT)
        mind_text = "--------------------------------\n---------------\n-----------------------\n--------------\n------------------"
        hi_there = Button(frame2, text=mind_text, command=say_hi)
        hi_there.pack()

        frame1.pack(padx=10, pady=10)
        frame2.pack(padx=200, pady=200)

        root.mainloop()
        remind_time = 45*60
        time.sleep(remind_time)


def demo_write_file(file_path):
    file = open("example.txt", "w")  # 打开文件以写入模式
    file.write("Hello, World!\n")
    file.write("This is a sample file.")
    file.close()


def nap_cmd_json_create():
    file = open("example.txt", "w")  # 打开文件以写入模式
    file.write("Hello, World!\n")
    file.write("This is a sample file.")
    file.close()


def show_notification(message, duration):
    toaster = ToastNotifier()
    toaster.show_toast("Notification", message, duration=duration)


def test_gui():
    # 创建定时器线程，每 5 秒执行一次 my_timer_function
    reminder_time = 1
    timer_thread = threading.Timer(reminder_time, my_timer_function)
    # 启动定时器线程
    timer_thread.start()
    while (1):
        time.sleep(10)


if __name__ == '__main__':
    test_gui()
