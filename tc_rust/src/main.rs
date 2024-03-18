use serde::{Serialize, Deserialize};
use serde_json::{Result, Value, Error};

#[derive(Serialize, Deserialize, Debug)]
struct NapCmd {
    bt: String,
    sbt: String,
    qtype: String,
    rr: Vec<String>,
}

//{"bt":"dns-force", "sbt":"dn_limit", "qtype":"aaaa", "rr":["1.1.1.1", "2.2.2.2" ]  }


fn untyped_example() -> Result<()> {
    // Some JSON input data as a &str. Maybe this comes from the user.
    let data = r#"
        {
            "name": "John Doe",
            "age": 43,
            "phones": [
                "+44 1234567",
                "+44 2345678"
            ]
        }"#;

    // Parse the string of data into serde_json::Value.
    let v: Value = serde_json::from_str(data)?;

    // Access parts of the data by indexing with square brackets.
    println!("Please call {} at the number {}", v["name"], v["phones"][0]);

    Ok(())
}

fn test() 
{
    let nap_cmd =  NapCmd {
        bt: String::from("how to bt with json in Rust"),
        sbt: String::from("how to sbt with json in Rust"),
        qtype: String::from("how to qtype with json in Rust"),
        rr: vec![String::from("how to 1 with json in Rust"),   String::from("how to 2 with json in Rust")],
    };

    let json = serde_json::to_string(&nap_cmd).unwrap();

    let json = String::from("099909809jlkkli09iomnlkjmoiu09");
    test_rust_json_nap_cmd(json);
    
    let json = serde_json::to_string(&nap_cmd).unwrap();

    println!("the JSON is: {}", json);
}



#[no_mangle]
pub extern "C" fn test_rust_json_nap_cmd(serialized: String) -> i32 {
    //let deserialized: Result<NapCmd, serde_json::Error> = serde_json::from_str(&json_str);
    //let deserialized: Result<NapCmd, serde_json::Error> = serde_json::from_str::<NapCmd>(&json_str);
    //println!("Error: {}", "try deserialized");

    let result: Result<NapCmd> = serde_json::from_str(&serialized);
    //rintln!("Deserialized point: {:?}", deserialized);
    match result {
        Ok(point) => {
            // 解析成功，继续处理点对象
            println!("Deserialized point: {:?}", point);
            return 0
        },
        Err(error) => {
            // 解析失败，处理错误信息
            println!("Error: {}", error);
            return 1;
        }
    }
    return 0;
}

fn main() {
    println!("Hello, world!");
    test();
}