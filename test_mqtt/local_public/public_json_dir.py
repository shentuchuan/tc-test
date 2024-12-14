# coding=utf-8

import os
import json


class JsonDir(object):
    """
    A class to read mqtt JSON files in a directory.
    """

    def __init__(self, path):
        self.path = path
        self.json_files = []
        self.json_data = []
        self.json_data_dict = {}

    def read_json_files_as_strings(self):
        """
        Read all JSON files in the directory as strings.
        """
        for file in os.listdir(self.path):
            if file.endswith(".json"):
                self.json_files.append(file)
                with open(os.path.join(self.path, file), 'r', encoding='utf-8') as f:
                    self.json_data.append(f.read())

    
