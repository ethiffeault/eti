//  MIT License
//  
//  Copyright (c) 2024 Eric Thiffeault
//  
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#include <iostream>

#include "doctest.h"

#include <eti/eti.h>

#if !ETI_SLIM_MODE

using namespace eti;

////////////////////////////////////////////////////////////////////////////////
namespace doc_introduction
{
    using namespace eti;

    struct Point
    {
        ETI_STRUCT(
            Point, 
            ETI_PROPERTIES( 
                ETI_PROPERTY( X ), 
                ETI_PROPERTY( Y ) ),
            ETI_METHODS( 
                ETI_METHOD( SetX ), 
                ETI_METHOD( Add ) ) )

        void SetX(int x)
        {
            X = x;
        }

        static Point Add(const Point& p0, const Point& p1)
        {
            return { p0.X + p1.X, p0.Y + p1.Y };
        }

        int X = 0;
        int Y = 0;
    };

    TEST_CASE("doc_introduction")
    {
        const Type& type = TypeOf<Point>();

        // set value using property
        {

            const Property* propertyX = type.GetProperty("X");
            Point p{ 1, 1 };
            propertyX->Set(p, 2);

            // stout: p.x = 2
            std::cout << "p.x = " << p.X << std::endl;       
        }

        // call SetX
        {
            const Method* set = type.GetMethod("SetX");
            Point p{ 1, 1 };
            int value = 101;
            set->CallMethod(p, (void*)nullptr, &value);


            // stout: p.x = 101
            std::cout << "p.x = " << p.X << std::endl;       
        }

        // call static method Add
        {
            const Method* add = type.GetMethod("Add");
            Point p1{ 1, 1 };
            Point p2{ 2, 2 };
            Point result;
            add->CallStaticMethod(&result, &p1, &p2);

            // stout: p1 + p2 = {3, 3}
            std::cout << "p1 + p2 = {" << result.X << ", " << result.Y << "}" << std::endl; 
        }
    }
}

#endif // #if !ETI_SLIM_MODE