#[cfg(test)]

mod tests {
    use std::{any::Any, borrow::BorrowMut, collections::HashMap};

    #[test]
    fn let_expressions() {
        let x = [1, 2, 3];
        if let Some(&found) = x.iter().find(|&x| *x == 2) {
            assert!(found == 2);
        }
    }

    #[test]
    fn binding() {
        let dict = HashMap::from([(1, "one"), (2, "two"), (3, "buckle my shoe")]);
        assert!(dict.contains_key(&1));

        let (x, _y) = dict.get_key_value(&3).unwrap();
        assert_eq!(*x, 3);
    }

    #[test]
    fn enums() {
        #[derive(PartialEq, Debug)]
        enum RustEnum {
            Int(i32),
            Double(f32),
            String(String),
        }

        impl Default for RustEnum {
            fn default() -> Self {
                RustEnum::Int(0)
            }
        }

        // Default behaviour needs to be implemented manually
        let mut rust_enum = RustEnum::default();
        assert!(rust_enum == RustEnum::Int(0));

        rust_enum = RustEnum::Double(7.0);
        assert!(rust_enum == RustEnum::Double(7.0));

        assert_ne!(rust_enum, RustEnum::Int(0));

        rust_enum = RustEnum::Int(8);
        assert_eq!(rust_enum, RustEnum::Int(8));

        rust_enum = RustEnum::String("Alin".to_string());
        assert_eq!(rust_enum, RustEnum::String("Alin".to_string()));

        match rust_enum {
            RustEnum::Int(value) => println!("int: {:#}", value),
            RustEnum::Double(value) => println!("double: {:#}", value),
            RustEnum::String(value) => println!("string: {:#}", value),
        }
    }

    #[test]
    fn option() {
        let mut option: Option<i32> = Default::default();

        assert_eq!(option, None);

        option = Some(5);
        assert_eq!(option, Some(5));

        // Won't compile as it doesn't decay to i32
        //*option.as_mut().unwrap() = 22.0;

        *option.as_mut().unwrap() = 22;
        assert_eq!(option, Some(22));

        option = None;
        assert_eq!(option.unwrap_or(5), 5);
    }

    #[test]
    fn boxes() {
        let mut any_obj: Box<dyn Any> = Box::new(1);
        println!("{:?}", (any_obj).type_id());

        assert_eq!(*any_obj.downcast::<i32>().unwrap(), 1);

        any_obj = Box::new("Alin");
        assert_eq!(*any_obj.downcast::<&str>().unwrap(), "Alin");
    }

    #[test]
    fn string_slices() {
        let str = " This 虎老 is a test\n";

        assert_eq!(str.chars().last().unwrap(), '\n');

        for c in str.chars() {
            print!("{}", c);
        }

        assert_eq!(str.find("This"), Some(1));
        assert_eq!(str.rfind('t'), Some(str.len() - 2));

        assert_eq!(str.find("虎"), Some(6));
        assert_eq!(str.find("老"), Some(9));

        assert_eq!(str.find("x"), None);
    }

    #[test]
    fn closures() {
        let func = |x| println!("closure {}", x);
        func(5);

        struct S {
            v: i32,
        }

        impl S {
            fn do_something(&self, x: i32) {
                println!("do_something {} {}", x, self.v)
            }
        }

        impl Default for S {
            fn default() -> Self {
                Self {
                    v: Default::default(),
                }
            }
        }

        let s: S = Default::default();
        S::do_something(&s, 5);

        let mut t: S = Default::default();
        t.v = 8;
        S::do_something(&t, 9);

        *t.v.borrow_mut() = 42;
        assert_eq!(t.v, 42);

        struct X {
            print_param: fn(i32),
        }

        impl Default for X {
            fn default() -> Self {
                Self {
                    print_param: |_: i32| {},
                }
            }
        }

        let mut x: X = Default::default();
        // Won't compile if Default trait is not implemented
        //(x.print_param)(4);
        x.print_param = |v: i32| println!("Called with: {}", v);
        (x.print_param)(4);
    }

    #[test]
    fn forward() {
        macro_rules! foo {
            ($($args:expr),*) => {{
                $(
                    println!("{:#?}", $args);
                )*
            }}
        }

        foo!(42, 'a', 4.2);
    }

    #[test]
    fn bits() {
        let a = 0b0;
        let b = 0xF;

        assert_eq!(b, 0xF);
        assert_eq!(b << 1, 0xF * 2);

        let c = a & b;
        assert_eq!(c, 0);

        let d = a | a;
        assert_eq!(d, 0);
    }

    #[test]
    fn entry() {
        let mut src = HashMap::from([(1, "one"), (2, "two"), (3, "buckle my shoe")]);
        let mut dst = HashMap::from([(4, "three")]);

        let rem_entry_1 = src.remove_entry(&1).unwrap();
        dst.insert(rem_entry_1.0, rem_entry_1.1);
        assert_eq!(src.get(&1), None);
        assert_eq!(dst.get(&1), Some(&"one"));

        let rem_entry_2 = src.remove_entry(&2).unwrap();
        dst.insert(rem_entry_2.0, rem_entry_2.1);
        let mut entry_2 = dst.remove_entry(&2).unwrap();
        assert_eq!(entry_2.0, 2);
        assert_eq!(entry_2.1, "two");

        entry_2.0 = 4;
        src.insert(entry_2.0, entry_2.1);
        assert!(src.get(&4) == Some(&"two"));

        dst.insert(entry_2.0, entry_2.1);
        assert!(src.get(&4) == Some(&"two"));
    }

    #[test]
    fn aggregate() {
        let a = [1, 2, 3];
        assert_eq!(a.iter().sum::<i32>(), 6);
        assert_eq!(a.into_iter().fold(0, |acc, v| { acc + v }), 6);

        assert_eq!(a.into_iter().fold(0, |acc, v| { acc + v * v }), 14);
        assert_eq!(a.into_iter().fold(1, |acc, v| { acc * v * v }), 36);

    }
}
