#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import unittest
import json
from subprocess import Popen, PIPE

class TestArion(unittest.TestCase):

  ARION_PATH = '../../build/arion'
  
  # Images for general purpose testing (leave off file:// for testing)
  IMAGE_1_PATH = '../../examples/images/image-1.jpg'
  IMAGE_2_PATH = '../../examples/images/image-2.jpg'
  IMAGE_3_PATH = '../../examples/images/image-3.jpg'
  
  # Images for JPG orientation tests (include file:// for testing)
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
  
  OUTPUT_IMAGE_PATH = 'output/'

  # -------------------------------------------------------------------------------
  #  Helper function for calling Arion
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
  #  Helper function for reading data back about an image
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
  #  Helper function for copying an image
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
  #  Helper function for checking for successful output
  # -------------------------------------------------------------------------------
  def verifySuccess(self, output, expected_width=-1, expected_height=-1):

    self.assertTrue(output['result'])
    self.assertEqual(output['failed_operations'], 0)
    self.assertEqual(output['total_operations'], 1)
    
    if expected_width >= 0:
      self.assertEqual(output['width'], expected_width)
      
    if expected_height >= 0:
      self.assertEqual(output['height'], expected_height)
  # -------------------------------------------------------------------------------
  #  Helper function for checking for failed output
  # -------------------------------------------------------------------------------
  def verifyFailure(self, output):
    self.assertFalse(output['result'])
    self.assertEqual(output['failed_operations'], 1)
    self.assertEqual(output['total_operations'], 1)
 
  # -------------------------------------------------------------------------------
  #  Helper function for creating output url
  # -------------------------------------------------------------------------------
  def outputUrlHelper(self, filename):
    return self.OUTPUT_IMAGE_PATH + filename
      
  # -------------------------------------------------------------------------------
  #  Helper function for testing fill operation
  # -------------------------------------------------------------------------------
  def imageResizeHelper(self, srcPath, outputPrefix, options):

    outputFilename = outputPrefix + \
                     str(options['width']) + 'x' + str(options['height']) + \
                     '_' + str(options['type']) + '.jpg'
    
    outputUrl = self.outputUrlHelper(outputFilename)
    
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':      options['width'],
        'height':     options['height'],
        'type':       options['type'],
        'gravity':    options['gravity'],
        'output_url': outputUrl
      }
    }

    operations = [resize_operation];

    output = self.call_arion(srcPath, operations)
    
    self.verifySuccess(output);

    #-----------------------------
    #  Now read back image data
    #-----------------------------

    output = self.read_image(outputUrl)

    self.verifySuccess(output, options['width'], options['height']);
    
  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------  
  def testImageFormats(self):
    
    # -----------------------------------------
    #                  JPG
    # -----------------------------------------
    input_url     = '../images/small_input.jpg'
    watermark_url = '../images/watermark.png'
    output_url    = self.outputUrlHelper('test_format_jpg.jpg')
    
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':            100,
        'height':           400,
        'type':             'width',
        'quality':          92,
        'watermark_url':    watermark_url,
        'watermark_type':   'adaptive',
        'watermark_min':    0.3,
        'watermark_max':    1.0,
        'output_url':       output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(input_url, operations)
    
    self.verifySuccess(output);
    
    # -----------------------------------------
    #                  PNG
    # -----------------------------------------
    input_url     = '../images/small_input.png'
    watermark_url = '../images/watermark.png'
    output_url    = self.outputUrlHelper('test_format_png.jpg')
    
    
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':            100,
        'height':           400,
        'type':             'width',
        'quality':          92,
        'watermark_url':    watermark_url,
        'watermark_type':   'adaptive',
        'watermark_min':    0.3,
        'watermark_max':    1.0,
        'output_url':       output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(input_url, operations)
    
    self.verifySuccess(output);
    
    # -----------------------------------------
    #                  TIFF
    # -----------------------------------------
    input_url     = '../images/small_input.tif'
    watermark_url = '../images/watermark.png'
    output_url    = self.outputUrlHelper('test_format_tif.jpg')
    
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':            100,
        'height':           400,
        'type':             'width',
        'quality':          92,
        'watermark_url':    watermark_url,
        'watermark_type':   'adaptive',
        'watermark_min':    0.3,
        'watermark_max':    1.0,
        'output_url':       output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(input_url, operations)
    
    self.verifySuccess(output);
    
  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------  
  def testWatermark(self):
    
    # -----------------------------------------
    #               Standard 1:1
    # -----------------------------------------
    input_url     = '../images/watermark_test_input.jpg'
    watermark_url = '../images/watermark.png'
    output_url    = self.outputUrlHelper('test_watermark_1_standard.jpg')
    
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':            400,
        'height':           400,
        'type':             'fill',
        'quality':          92,
        'watermark_url':    watermark_url,
        'watermark_type':   'standard',
        'watermark_amount': 0.1,
        'output_url':       output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(input_url, operations)
    
    self.verifySuccess(output);
    
    # -----------------------------------------
    #              Adaptive 1:1
    # -----------------------------------------
    input_url     = '../images/watermark_test_input.jpg'
    watermark_url = '../images/watermark.png'
    output_url    = self.outputUrlHelper('test_watermark_2_adaptive.jpg')
    
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':            400,
        'height':           400,
        'type':             'fill',
        'quality':          92,
        'watermark_url':    watermark_url,
        'watermark_type':   'adaptive',
        'watermark_min':    0.1,
        'watermark_max':    0.5,
        'output_url':       output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(input_url, operations)
    
    self.verifySuccess(output);
    
    # -----------------------------------------
    #   Output size is smaller than watermark
    # -----------------------------------------
    watermark_url = '../images/watermark2.png'
    output_url    = self.outputUrlHelper('test_watermark_2_photo.jpg')
    
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':            200,
        'height':           200,
        'type':             'fill',
        'quality':          92,
        'watermark_url':    watermark_url,
        'watermark_type':   'adaptive',
        'watermark_min':    0.1,
        'watermark_max':    0.5,
        'output_url':       output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(self.IMAGE_1_PATH, operations)
    
    self.verifySuccess(output);
    
    # -----------------------------------------
    #   Output size is larger than watermark
    # -----------------------------------------
    watermark_url = '../images/watermark2.png'
    output_url    = self.outputUrlHelper('test_watermark_3_photo.jpg')
    
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':            1000,
        'height':           1000,
        'type':             'fill',
        'quality':          92,
        'watermark_url':    watermark_url,
        'watermark_type':   'adaptive',
        'watermark_min':    0.1,
        'watermark_max':    0.5,
        'output_url':       output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(self.IMAGE_1_PATH, operations)
    
    self.verifySuccess(output);
    
    # -----------------------------------------
    # Output width is larger than watermark, 
    # but height is smaller
    # -----------------------------------------
    output_url = self.outputUrlHelper('test_watermark_4_photo.jpg')
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':            1000,
        'height':           200,
        'type':             'fill',
        'quality':          92,
        'watermark_url':    watermark_url,
        'watermark_type':   'adaptive',
        'watermark_min':    0.1,
        'watermark_max':    0.5,
        'output_url':       output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(self.IMAGE_1_PATH, operations)
    
    self.verifySuccess(output);
    
    # -----------------------------------------
    # Output height is larger than watermark, 
    # but width is smaller
    # -----------------------------------------
    output_url = self.outputUrlHelper('test_watermark_5_photo.jpg')
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':            200,
        'height':           1000,
        'type':             'fill',
        'quality':          92,
        'watermark_url':    watermark_url,
        'watermark_url':    watermark_url,
        'watermark_type':   'adaptive',
        'watermark_min':    0.1,
        'watermark_max':    0.5,
        'output_url':       output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(self.IMAGE_1_PATH, operations)
    
    self.verifySuccess(output);

  # -------------------------------------------------------------------------------
  # Here we have a tall source image and we are always cropping a tall portion at
  # the center of the image
  # -------------------------------------------------------------------------------  
  def test100x200TallCenter(self):

    srcPath      = "file://../images/100x200_tall_center.png"
    outputPrefix = "100x200_tall_center_to_"

    # Just a crop, take the center
    opts = {
        'type':    'fill',
        'gravity': 'center',
        'width':   50,
        'height':  200,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)
    
    opts = {
        'type':    'fill',
        'gravity': 'north',
        'width':   25,
        'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)
    
    opts = {
        'type':    'fill',
        'gravity': 'south',
        'width':   100,
        'height':  400,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # ------------------------------------------------------------------------------- 
  # Here we have a tall source image and we are always cropping a tall portion at 
  # the left of the image
  # ------------------------------------------------------------------------------- 
  def test100x200TallLeft(self):
      
    srcPath      = "file://../images/100x200_tall_left.png"
    outputPrefix = "100x200_tall_left_to_"

    # Just a crop, take the left
    opts = {
      'type':    'fill',
      'gravity': 'west',
      'width':   50,
      'height':  200,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Shrink, take the left
    opts = {
      'type':    'fill',
      'gravity': 'northwest',
      'width':   25,
      'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Enlarge, take the left
    opts = {
      'type':    'fill',
      'gravity': 'southwest',
      'width':   100,
      'height':  400,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # ------------------------------------------------------------------------------- 
  # Here we have a tall source image and we are always cropping a tall portion
  # at the right of the image
  # ------------------------------------------------------------------------------- 
  def test100x200TallRight(self):

    srcPath      = "file://../images/100x200_tall_right.png"
    outputPrefix = "100x200_tall_right_to_"

    # Just a crop, take the right
    opts = {
      'type':    'fill',
      'gravity': 'east',
      'width':   50,
      'height':  200,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Shrink, take the right
    opts = {
      'type':    'fill',
      'gravity': 'northeast',
      'width':   25,
      'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Enlarge, take the right
    opts = {
      'type':    'fill',
      'gravity': 'southeast',
      'width':   100,
      'height':  400,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # ------------------------------------------------------------------------------- 
  # Here we have a tall source image and we are always cropping a wide portion
  # at the bottom of the image
  # ------------------------------------------------------------------------------- 
  def test100x200WideBottom(self):

    srcPath      = "file://../images/100x200_wide_bottom.png"
    outputPrefix = "100x200_wide_bottom_to_"

    # Just a crop, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'south',
      'width':   100,
      'height':  50,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Shrink, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'southeast',
      'width':   50,
      'height':  25,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Enlarge, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'southwest',
      'width':   200,
      'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # ------------------------------------------------------------------------------- 
  # Here we have a tall source image and we are always cropping a wide portion
  # at the bottom of the image
  # ------------------------------------------------------------------------------- 
  def test100x200WideCenter(self):

    srcPath      = "file://../images/100x200_wide_center.png"
    outputPrefix = "100x200_wide_center_to_"

    # Just a crop, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'center',
      'width':   100,
      'height':  50,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Shrink, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'east',
      'width':   50,
      'height':  25,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Enlarge, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'west',
      'width':   200,
      'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # ------------------------------------------------------------------------------- 
  # Here we have a tall source image and we are always cropping a wide portion
  # at the top of the image
  # ------------------------------------------------------------------------------- 
  def test100x200WideTop(self):

    srcPath      = "file://../images/100x200_wide_top.png"
    outputPrefix = "100x200_wide_top_to_"

    # Just a crop, take the top
    opts = {
      'type':    'fill',
      'gravity': 'north',
      'width':   100,
      'height':  50,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Shrink, take the top
    opts = {
      'type':    'fill',
      'gravity': 'northeast',
      'width':   50,
      'height':  25,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Enlarge, take the top
    opts = {
      'type':    'fill',
      'gravity': 'northwest',
      'width':   200,
      'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # ------------------------------------------------------------------------------- 
  # Here we have a wide source image and we are always cropping a tall portion at
  # the center of the image
  # ------------------------------------------------------------------------------- 
  def test200x100TallCenter(self):

    srcPath      = "file://../images/200x100_tall_center.png"
    outputPrefix = "200x100_tall_center_to_"

    # Just a crop, take the center
    opts = {
      'type':    'fill',
      'gravity': 'center',
      'width':   50,
      'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Shrink, take the center
    opts = {
      'type':    'fill',
      'gravity': 'north',
      'width':   25,
      'height':  50,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Enlarge, take the center
    opts = {
      'type':    'fill',
      'gravity': 'south',
      'width':   100,
      'height':  200,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # ------------------------------------------------------------------------------- 
  # Here we have a tall source image and we are always cropping a tall portion at
  # the left of the image
  # ------------------------------------------------------------------------------- 
  def test200x100TallLeft(self):

    srcPath      = "file://../images/200x100_tall_left.png"
    outputPrefix = "200x100_tall_left_to_"

    # Just a crop, take the left
    opts = {
      'type':    'fill',
      'gravity': 'west',
      'width':   50,
      'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Shrink, take the left
    opts = {
      'type':    'fill',
      'gravity': 'northwest',
      'width':   25,
      'height':  50,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Enlarge, take the left
    opts = {
      'type':    'fill',
      'gravity': 'southwest',
      'width':   100,
      'height':  200,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # ------------------------------------------------------------------------------- 
  # Here we have a tall source image and we are always cropping a tall portion at
  # the right of the image
  # ------------------------------------------------------------------------------- 
  def test200x100TallRight(self):

    srcPath      = "file://../images/200x100_tall_right.png"
    outputPrefix = "200x100_tall_right_to_"

    # Just a crop, take the right
    opts = {
      'type':    'fill',
      'gravity': 'east',
      'width':   50,
      'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Shrink, take the right
    opts = {
      'type':    'fill',
      'gravity': 'northeast',
      'width':   25,
      'height':  50,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Enlarge, take the right
    opts = {
      'type':    'fill',
      'gravity': 'southeast',
      'width':   100,
      'height':  200,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # ------------------------------------------------------------------------------- 
  # Here we have a tall source image and we are always cropping a wide portion at 
  # the bottom of the image
  # ------------------------------------------------------------------------------- 
  def test200x100WideBottom(self):

    srcPath      = "file://../images/200x100_wide_bottom.png"
    outputPrefix = "200x100_wide_bottom_to_"

    # Just a crop, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'south',
      'width':   200,
      'height':  50,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Shrink, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'southeast',
      'width':   100,
      'height':  25,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Enlarge, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'southwest',
      'width':   400,
      'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # ------------------------------------------------------------------------------- 
  # Here we have a tall source image and we are always cropping a wide portion at 
  # the bottom of the image
  # ------------------------------------------------------------------------------- 
  def test200x100WideCenter(self):

    srcPath      = "file://../images/200x100_wide_center.png"
    outputPrefix = "200x100_wide_center_to_"

    # Just a crop, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'center',
      'width':   200,
      'height':  50,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Shrink, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'east',
      'width':   100,
      'height':  25,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Enlarge, take the bottom
    opts = {
      'type':    'fill',
      'gravity': 'west',
      'width':   400,
      'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # ------------------------------------------------------------------------------- 
  # Here we have a wide source image and we are always cropping a wide portion at 
  # the top of the image
  # ------------------------------------------------------------------------------- 
  def test200x100WideTop(self):

    srcPath      = "file://../images/200x100_wide_top.png"
    outputPrefix = "200x100_wide_top_to_"

    # Just a crop, take the top
    opts = {
      'type':    'fill',
      'gravity': 'north',
      'width':   200,
      'height':  50,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Shrink, take the top
    opts = {
      'type':    'fill',
      'gravity': 'northeast',
      'width':   100,
      'height':  25,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

    # Enlarge, take the top
    opts = {
      'type':    'fill',
      'gravity': 'northwest',
      'width':   400,
      'height':  100,
    }
    self.imageResizeHelper(srcPath, outputPrefix, opts)

  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_basic_jpg_resize(self):

    #-----------------------------
    #       Resize image
    #-----------------------------
    output_url = self.outputUrlHelper('test_basic_jpg_resize.jpg')

    # Use low JPG quality to make sure parameter is working
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

    self.verifySuccess(output, 1296, 864);

    #-----------------------------
    #  Now read back image data
    #-----------------------------

    output = self.read_image(output_url)

    self.verifySuccess(output, 200, 133);

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
  def test_resize_shrink_width_limit(self):

    output_url = self.outputUrlHelper('test_resize_shrink_width_limit.jpg')

    operation = {
      'type': 'resize',
      'params':
      {
        'width':      200,
        'height':     120,
        'type':       'width',
        'quality':    92,
        'output_url': output_url
      }
    }
    
    output = self.call_arion(self.IMAGE_1_PATH, [operation])

    self.verifySuccess(output)

    #-----------------------------
    #  Now read back image data
    #-----------------------------

    output = self.read_image(output_url)

    self.verifySuccess(output, 180, 120)
    
  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_resize_shrink_height(self):

    output_url = self.outputUrlHelper('test_resize_shrink_height.jpg')

    operation = {
      'type': 'resize',
      'params':
      {
        'width':      1000,
        'height':     200,
        'type':       'height',
        'quality':    92,
        'output_url': output_url
      }
    }

    output = self.call_arion(self.IMAGE_1_PATH, [operation])

    self.verifySuccess(output);

    #-----------------------------
    #  Now read back image data
    #-----------------------------

    output = self.read_image(output_url)

    self.verifySuccess(output, 300, 200)

  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_resize_shrink_height_limit(self):

    output_url = self.outputUrlHelper('test_resize_shrink_height_limit.jpg')

    operation = {
      'type': 'resize',
      'params':
      {
        'width':      200,
        'height':     200,
        'type':       'height',
        'quality':    92,
        'output_url': output_url
      }
    }

    output = self.call_arion(self.IMAGE_1_PATH, [operation])

    self.verifySuccess(output);

    #-----------------------------
    #  Now read back image data
    #-----------------------------

    output = self.read_image(output_url)

    self.verifySuccess(output, 200, 133);

  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_resize_shrink_square(self):

    output_url = self.outputUrlHelper('test_resize_shrink_square.jpg')

    # Height should not matter here...
    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':      200,
        'height':     2000,
        'type':       'square',
        'quality':    92,
        'output_url': output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(self.IMAGE_1_PATH, operations)

    self.verifySuccess(output);

    #-----------------------------
    #  Now read back image data
    #-----------------------------

    output = self.read_image(output_url)

    self.verifySuccess(output, 200, 200);
    
  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_resize_fill(self):

    output_url = self.outputUrlHelper('test_resize_fill.jpg')

    resize_operation = {
      'type': 'resize',
      'params':
      {
        'width':      200,
        'height':     400,
        'type':       'fill',
        'quality':    92,
        'output_url': output_url
      }
    }

    operations = [resize_operation];

    output = self.call_arion(self.IMAGE_1_PATH, operations)

    self.verifySuccess(output);

    #-----------------------------
    #  Now read back image data
    #-----------------------------

    output = self.read_image(output_url)

    #self.verifySuccess(output, 200, 200);
    
  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_basic_read_meta(self):

    #-----------------------------
    #      Read image meta
    #-----------------------------
    output = self.read_image(self.IMAGE_1_PATH)

    self.verifySuccess(output, 1296, 864);

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

    output = self.copy_image(self.LANDSCAPE_1_PATH, self.outputUrlHelper('Landscape_1.jpg'))
    self.verifySuccess(output);
    
    output = self.copy_image(self.LANDSCAPE_2_PATH, self.outputUrlHelper('Landscape_2.jpg'))
    self.verifySuccess(output);
    
    output = self.copy_image(self.LANDSCAPE_3_PATH, self.outputUrlHelper('Landscape_3.jpg'))
    self.verifySuccess(output);
    
    output = self.copy_image(self.LANDSCAPE_4_PATH, self.outputUrlHelper('Landscape_4.jpg'))
    self.verifySuccess(output);

    output = self.copy_image(self.LANDSCAPE_5_PATH, self.outputUrlHelper('Landscape_5.jpg'))
    self.verifySuccess(output);
    
    output = self.copy_image(self.LANDSCAPE_6_PATH, self.outputUrlHelper('Landscape_6.jpg'))
    self.verifySuccess(output);
    
    output = self.copy_image(self.LANDSCAPE_7_PATH, self.outputUrlHelper('Landscape_7.jpg'))
    self.verifySuccess(output);
    
    output = self.copy_image(self.LANDSCAPE_8_PATH, self.outputUrlHelper('Landscape_8.jpg'))
    self.verifySuccess(output);

  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_invalid_operation(self):

    read_meta_operation = {
      'type': 'invalid',
      'params': {
        'value': 'bogus'
      }
    }

    operations = [read_meta_operation];

    output = self.call_arion(self.IMAGE_1_PATH, operations)
    
    self.assertFalse(output['result'])
    
  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_invalid_json(self):
    # Missing ending brace, but otherwise valid
    input_string = "{\"input_url\":\"file://../../examples/images/image-1.jpg\",\"correct_rotation\":true,\"operations\":[{\"type\":\"read_meta\",\"params\":{\"info\":true}}]"
    p = Popen([self.ARION_PATH, "--input", input_string], stdout=PIPE)

    cmd_output = p.communicate()

    output = json.loads(cmd_output[0])

    self.assertFalse(output['result'])
    
  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def testNoParams(self):
  
    # Missing params
    operation = {
      'type': 'read_meta'
    }
    
    output = self.call_arion(self.IMAGE_1_PATH, [operation])
    
    self.assertFalse(output['result'])
  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def test_md5(self):

    operation = {
      'type': 'fingerprint',
      'params':
        {
          'type':      'md5'
        }
    }

    operations = [operation];
    output = self.call_arion(self.IMAGE_1_PATH, operations)

    self.assertEqual(output['info'][0]['md5'], 'c8d342a627da420e77c2e90a10f75689')
  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def testInvalidCopyParams(self):
  
    # No output_url
    operation = {
      'type': 'copy',
      'params': {
      }
    }
    self.verifyFailure(self.call_arion(self.IMAGE_1_PATH, [operation]))
    
    # Empty output_url
    operation = {
      'type': 'copy',
      'params': {
        'output_url': ''
      }
    }
    self.verifyFailure(self.call_arion(self.IMAGE_1_PATH, [operation]))
  
    # Missing valid output_url
    operation = {
      'type': 'copy',
      'params': {
        'output_url': ''
      }
    }
    self.verifyFailure(self.call_arion(self.IMAGE_1_PATH, [operation]))

  # -------------------------------------------------------------------------------
  # -------------------------------------------------------------------------------
  def testInvalidResizeParams(self):
  
    # Resize operation missing type
    operation = {
      'type': 'resize',
      'params':
      {
        'width':      200,
        'height':     400,
        'output_url': 'output.jpg'
      }
    }
    self.verifyFailure(self.call_arion(self.IMAGE_1_PATH, [operation]))
    
    # Resize operation missing width
    operation = {
      'type': 'resize',
      'params':
      {
        'type':       'width',
        'height':     400,
        'output_url': 'output.jpg'
      }
    }
    self.verifyFailure(self.call_arion(self.IMAGE_1_PATH, [operation]))
    
    # Resize operation missing height
    operation = {
      'type': 'resize',
      'params':
      {
        'type':       'width',
        'width':      200,
        'output_url': 'output.jpg'
      }
    }
    self.verifyFailure(self.call_arion(self.IMAGE_1_PATH, [operation]))

    # Desire size too big
    operation = {
      'type': 'resize',
      'params':
        {
          'type': 'width',
          'width': 10000,
          'height': 10001,
          'output_url': 'output.jpg'
        }
    }
    self.verifyFailure(self.call_arion(self.IMAGE_1_PATH, [operation]))

  # -------------------------------------------------------------------------------
  #  Called only once
  # -------------------------------------------------------------------------------
  @classmethod
  def setUpClass(cls):
    if not os.path.exists(cls.OUTPUT_IMAGE_PATH):
      os.makedirs(cls.OUTPUT_IMAGE_PATH)
    
    # Remove any existing output files
    for the_file in os.listdir(cls.OUTPUT_IMAGE_PATH):
      file_path = os.path.join(cls.OUTPUT_IMAGE_PATH, the_file)
      
      try:
        if os.path.isfile(file_path):
          os.unlink(file_path)
      except Exception as e:
        print(e)
        
# -------------------------------------------------------------------------------
# -------------------------------------------------------------------------------
if __name__ == '__main__':
    unittest.main()
