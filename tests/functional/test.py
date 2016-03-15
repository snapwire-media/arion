#!/usr/bin/python
# -*- coding: utf-8 -*-

import unittest
import json
from subprocess import Popen, PIPE

class TestArion(unittest.TestCase):

  ARION_PATH = '../../build/arion'
  IMAGE_1_PATH = 'file://../../examples/images/image-1.jpg'
  IMAGE_2_PATH = 'file://../../examples/images/image-2.jpg'
  IMAGE_3_PATH = 'file://../../examples/images/image-3.jpg'

  # -------------------------------------------------------------------------------
  # Helper function for calling Arion
  # -------------------------------------------------------------------------------
  def call_arion(self, input_url, operations):

    input_dict = {'input_url':        input_url,
                  'correct_rotation': True,
                  'operations':       operations}

    input_string = json.dumps(input_dict, separators=(',', ':'))

    p = Popen([self.ARION_PATH, "--input", input_string], stdout=PIPE)

    cmd_output = p.communicate()

    output = json.loads(cmd_output[0])

    # DEBUG
    # print cmd_output[0]

    return output

  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_basic_resize(self):

    #-----------------------------
    #       Resize image
    #-----------------------------
    output_url = 'file://output.jpg'

    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':      100,
        'height':     1000,
        'type':       'width',
        'quality':    85,
        'output_url': output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(self.IMAGE_1_PATH, operations)

    self.assertTrue(output['result'])
    self.assertEqual(output['failed_operations'], 0)
    self.assertEqual(output['total_operations'], 1)
    self.assertEqual(output['height'], 864)
    self.assertEqual(output['width'], 1296)
    self.assertEqual(output['md5'], 'c8d342a627da420e77c2e90a10f75689')

    #-----------------------------
    #  Now read back image data
    #-----------------------------

    read_meta_operation = {
      'type': 'read_meta',
      "params": {
        "info": True
      }
    }

    operations = [read_meta_operation];

    output = self.call_arion(output_url, operations)

    self.assertTrue(output['result'])
    self.assertEqual(output['failed_operations'], 0)
    self.assertEqual(output['total_operations'], 1)
    self.assertEqual(output['height'], 67)
    self.assertEqual(output['width'], 100)
    self.assertEqual(output['md5'], '4ec4ec2abde005187852424a153d4c48')

    info = output['info'][0]
    self.assertTrue(info['result'])
    self.assertEqual(info['type'], 'read_meta')

    # By default meta data gets stripped
    self.assertFalse(info['model_released'])
    self.assertFalse(info['property_released'])
    self.assertEqual(info['special_instructions'], '')
    self.assertEqual(info['subject'], '')
    self.assertEqual(info['copyright'], '')
    self.assertEqual(info['city'], '')
    self.assertEqual(info['province_state'], '')
    self.assertEqual(info['country_name'], '')
    self.assertEqual(info['country_code'], '')
    self.assertEqual(info['caption'], '')
    self.assertEqual(info['keywords'], [])

  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_basic_read_meta(self):

    #-----------------------------
    #       Resize image
    #-----------------------------
    read_meta_operation = {
      'type': 'read_meta',
      "params": {
        "info": True
      }
    }

    operations = [read_meta_operation];

    output = self.call_arion(self.IMAGE_1_PATH, operations)

    self.assertTrue(output['result'])
    self.assertEqual(output['failed_operations'], 0)
    self.assertEqual(output['total_operations'], 1)
    self.assertEqual(output['height'], 864)
    self.assertEqual(output['width'], 1296)
    self.assertEqual(output['md5'], 'c8d342a627da420e77c2e90a10f75689')

    info = output['info'][0]
    self.assertTrue(info['result'])
    self.assertEqual(info['type'], 'read_meta')

    # By default meta data gets stripped
    self.assertFalse(info['model_released'])
    self.assertFalse(info['property_released'])
    self.assertEqual(info['special_instructions'], 'Not Released (NR)')
    self.assertEqual(info['subject'], '')
    self.assertEqual(info['copyright'], 'Paul Filitchkin')
    self.assertEqual(info['city'], 'Bol')
    # TODO
    #self.assertEqual(info['province_state'], "Splitsko-dalmatinska županija")
    self.assertEqual(info['country_name'], 'Croatia')
    self.assertEqual(info['country_code'], 'HR')
    self.assertEqual(info['caption'], 'Windy road during sunset on Brac Island in Croatia - "Republic of Croatia"')

    keywords = info['keywords']

    self.assertTrue("Adriatic Sea" in keywords)
    self.assertTrue("Balkans" in keywords)
    self.assertTrue("Croatia" in keywords)
    self.assertTrue("Europe" in keywords)
    self.assertTrue("island" in keywords)
    self.assertTrue("outdoors" in keywords)
    self.assertTrue("road" in keywords)
    self.assertTrue("roadtrip" in keywords)
    self.assertTrue("sea" in keywords)
    self.assertTrue("sunset" in keywords)

# -------------------------------------------------------------------------------
# -------------------------------------------------------------------------------
if __name__ == '__main__':
    unittest.main()
