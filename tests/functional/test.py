#!/usr/bin/python
# -*- coding: utf-8 -*-

import unittest
import json
from subprocess import Popen, PIPE

class TestArion(unittest.TestCase):

  ARION_PATH = '../../build/arion'
  
  # Images for general purpose testing
  IMAGE_1_PATH = 'file://../../examples/images/image-1.jpg'
  IMAGE_2_PATH = 'file://../../examples/images/image-2.jpg'
  IMAGE_3_PATH = 'file://../../examples/images/image-3.jpg'
  
  # Images for JPG orientation tests
  # Images from https://github.com/recurser/exif-orientation-examples
  # Copyright (c) 2010 Dave Perrett.
  LANDSCAPE_1_PATH = 'file://../images/Landscape_1.jpg'
  LANDSCAPE_2_PATH = 'file://../images/Landscape_2.jpg'
  LANDSCAPE_3_PATH = 'file://../images/Landscape_3.jpg'
  LANDSCAPE_4_PATH = 'file://../images/Landscape_4.jpg'
  LANDSCAPE_5_PATH = 'file://../images/Landscape_5.jpg'
  LANDSCAPE_6_PATH = 'file://../images/Landscape_6.jpg'
  LANDSCAPE_7_PATH = 'file://../images/Landscape_7.jpg'
  LANDSCAPE_8_PATH = 'file://../images/Landscape_8.jpg'

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
  # Helper function for reading data back about an image
  # -------------------------------------------------------------------------------
  def read_image(self, input_url):

    operation = {
      'type': 'read_meta',
      'params': {
        'info': True
      }
    }
    
    return self.call_arion(input_url, [operation])
    
  # -------------------------------------------------------------------------------
  # Helper function for copying an image
  # -------------------------------------------------------------------------------
  def copy_image(self, input_url, output_url):

    operation = {
      'type': 'copy',
      'params': {
        'output_url': output_url
      }
    }
    
    return self.call_arion(input_url, [operation])
    
  # -------------------------------------------------------------------------------
  # Helper function for copying an image
  # -------------------------------------------------------------------------------
  def verify_success(self, output):

    self.assertTrue(output['result'])
    self.assertEqual(output['failed_operations'], 0)
    self.assertEqual(output['total_operations'], 1)
    
  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_basic_jpg_resize(self):

    #-----------------------------
    #       Resize image
    #-----------------------------
    output_url = 'file://output.jpg'

    # Use low quality to make sure parameter is working
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':      200,
        'height':     1000,
        'type':       'width',
        'quality':    50,
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
    #self.assertEqual(output['md5'], 'c8d342a627da420e77c2e90a10f75689')

    #-----------------------------
    #  Now read back image data
    #-----------------------------

    output = self.read_image(output_url)

    self.assertTrue(output['result'])
    self.assertEqual(output['failed_operations'], 0)
    self.assertEqual(output['total_operations'], 1)
    self.assertEqual(output['height'], 133)
    self.assertEqual(output['width'], 200)
    #self.assertEqual(output['md5'], '4ec4ec2abde005187852424a153d4c48')

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
    #      Read image meta
    #-----------------------------
    output = self.read_image(self.IMAGE_1_PATH)

    self.assertTrue(output['result'])
    self.assertEqual(output['failed_operations'], 0)
    self.assertEqual(output['total_operations'], 1)
    self.assertEqual(output['height'], 864)
    self.assertEqual(output['width'], 1296)
    #self.assertEqual(output['md5'], 'c8d342a627da420e77c2e90a10f75689')

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
  def test_jpg_orienation(self):

    output = self.copy_image(self.LANDSCAPE_1_PATH, 'file://Landscape_1.jpg')
    self.verify_success(output);
    
    output = self.copy_image(self.LANDSCAPE_2_PATH, 'file://Landscape_2.jpg')
    self.verify_success(output);
    
    output = self.copy_image(self.LANDSCAPE_3_PATH, 'file://Landscape_3.jpg')
    self.verify_success(output);
    
    output = self.copy_image(self.LANDSCAPE_4_PATH, 'file://Landscape_4.jpg')
    self.verify_success(output);

    output = self.copy_image(self.LANDSCAPE_5_PATH, 'file://Landscape_5.jpg')
    self.verify_success(output);
    
    output = self.copy_image(self.LANDSCAPE_6_PATH, 'file://Landscape_6.jpg')
    self.verify_success(output);
    
    output = self.copy_image(self.LANDSCAPE_7_PATH, 'file://Landscape_7.jpg')
    self.verify_success(output);
    
    output = self.copy_image(self.LANDSCAPE_8_PATH, 'file://Landscape_8.jpg')
    self.verify_success(output);

  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_invalid_operation(self):

    read_meta_operation = {
      'type': 'invalid',
      "params": {
        "value": "bogus"
      }
    }

    operations = [read_meta_operation];

    output = self.call_arion(self.IMAGE_1_PATH, operations)
    
    self.assertFalse(output['result'])
    
  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_no_params(self):
  
    # Missing params
    operation = {
      'type': 'read_meta'
    }
    
    output = self.call_arion(self.IMAGE_1_PATH, [operation])
    
    self.assertFalse(output['result'])

# -------------------------------------------------------------------------------
# -------------------------------------------------------------------------------
if __name__ == '__main__':
    unittest.main()
