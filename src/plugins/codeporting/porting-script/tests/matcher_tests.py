"""
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
"""
import sys
import unittest

# exec from root dirctory
sys.path.append('.') 

from tool.match.match_factory import MatcherFactory
from tool.tool_config import MatcherType


class TestMatcher(unittest.TestCase):
    def setUp(self):
        self.file_type_dict = {}
        self.file_type_dict["cppfiles"] = ["./cpp/ice_rxtx_vec_avx2.c"]
        mather_factory = MatcherFactory(self.file_type_dict)
        self.matcher = mather_factory.get_matcher(MatcherType.CppMatcher)

    def test_cpp(self):
        results = self.matcher.match()
        # print(results.file_path)
        self.assertGreaterEqual(len(results), 1)

    def test_macro_branch_judge(self):
        """[['if', 308, '#if __x86__', 'x86'], ['elif', 310, '#elif aarch64', 'arm'], ['elif', 312,
        #elif mips', 'mips'], ['endif', 316, '#endif', 'all']]"""
        input = [['if', 308, '#if __x86__', 'x86'], ['elif', 310, '#elif aarch64', 'arm'],
                 ['elif', 312, '#elif mips', 'mips'], ['endif', 316, '#endif', 'all']]
        start, end, need = self.matcher.macro_branch_judge(input)
        self.assertEqual(need, 0)

        input = [['if', 308, '#if __x86__', 'x86'], ['elif', 310, '#elif shenwei', 'all'],
                 ['elif', 312, '#elif mips', 'mips'], ['endif', 316, '#endif', 'all']]
        start, end, need = self.matcher.macro_branch_judge(input)
        self.assertEqual(need, 1)

        input = [['ifndef', 308, '#ifndef arrch64', 'all'], ['elif', 310, '#elif aarch64', 'arm'],
                 ['elif', 312, '#elif mips', 'mips'], ['endif', 316, '#endif', 'all']]
        start, end, need = self.matcher.macro_branch_judge(input)
        self.assertEqual(need, 0)

if __name__ == "__main__":
    # create test suite
    suite = unittest.TestSuite()
    # add test case
    tests = [TestMatcher("test_cpp"), TestMatcher("test_macro_branch_judge")]
    suite.addTests(tests)
    # run
    runner = unittest.TextTestRunner()
    runner.run(suite)